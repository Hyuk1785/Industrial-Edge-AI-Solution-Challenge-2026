# train.py
import numpy as np
import pandas as pd
import pickle
import os
from tensorflow import keras
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from utils import file_to_windows

# ===== 사용자 설정 =====
TRAIN_NORMAL_FILES = [
    "data/normal_20260223_213852_part1.csv"   # ← 새 파일
]

TRAIN_DEFECT_FILES = [
    "data/defect_20260223_221151_part1.csv"   # ← 새 파일
]
# ======================

os.makedirs("model", exist_ok=True)

train_features, train_labels = [], []

for f in TRAIN_NORMAL_FILES:
    feats, labs, _ = file_to_windows(f, label=0)
    train_features += feats
    train_labels += labs

for f in TRAIN_DEFECT_FILES:
    feats, labs, _ = file_to_windows(f, label=1)
    train_features += feats
    train_labels += labs

X = pd.DataFrame(train_features).values.astype(np.float32)
y = np.array(train_labels, dtype=np.float32)

print(f"전체 윈도우 수: {len(X)}")
print(f"정상: {int((y==0).sum())}, 결함: {int((y==1).sum())}")

# [수정1] train/validation 분리 추가
X_train, X_val, y_train, y_val = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

# 정규화
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_val   = scaler.transform(X_val)   # ← val은 fit 하지 않음

pickle.dump(scaler, open("model/scaler.pkl", "wb"))
np.save("model/scaler_mean.npy", scaler.mean_)
np.save("model/scaler_std.npy", scaler.scale_)

print(f"피처 수: {X_train.shape[1]}")
print(f"scaler mean: {scaler.mean_}")
print(f"scaler std:  {scaler.scale_}")

# [수정2] 클래스 가중치 추가 (불균형 대비)
class_weight = {
    0: 1.0,
    1: 1.0  # 데이터 균형 맞으면 1.0, 불균형하면 조정
}

# 모델 (피처 수가 자동으로 맞춰짐)
model = keras.Sequential([
    keras.layers.Input(shape=(X_train.shape[1],)),
    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dropout(0.5),
    keras.layers.Dense(16, activation='relu'),
    keras.layers.Dropout(0.4),
    keras.layers.Dense(8, activation='relu'),
    keras.layers.Dense(1, activation='sigmoid'),
])

model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)

model.summary()

history = model.fit(
    X_train, y_train,
    epochs=30,
    batch_size=16,         # [수정3] 데이터 작으니 batch_size 줄임
    validation_data=(X_val, y_val),
    class_weight=class_weight,
    callbacks=[
        keras.callbacks.EarlyStopping(
            monitor='val_loss',
            patience=5,
            restore_best_weights=True
            
        )
    ]
)

# 검증 결과 출력
val_loss, val_acc = model.evaluate(X_val, y_val, verbose=0)
print(f"\n✅ Validation Accuracy: {val_acc*100:.1f}%")
print(f"✅ Validation Loss: {val_loss:.4f}")

# 임계값 분석
y_pred = model.predict(X_val).flatten()
for threshold in [0.1, 0.3, 0.5]:
    pred_labels = (y_pred > threshold).astype(int)
    acc = (pred_labels == y_val).mean()
    print(f"  threshold={threshold}: accuracy={acc*100:.1f}%")

model.save("model/motor_model.h5")
print("\n✅ 학습 완료 및 모델 저장")