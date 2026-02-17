# train.py
import numpy as np
import pandas as pd
import pickle
from tensorflow import keras
from sklearn.preprocessing import StandardScaler
from utils import file_to_windows

# ===== 사용자 설정 =====
TRAIN_NORMAL_FILES = [
    "data/normal_20260217_180758.csv"
]

TRAIN_DEFECT_FILES = [
    "data/defect_20260217_182038.csv"
]

# ======================

train_features, train_labels = [], []

for f in TRAIN_NORMAL_FILES:
    feats, labs, _ = file_to_windows(f, label=0)
    train_features += feats
    train_labels += labs

for f in TRAIN_DEFECT_FILES:
    feats, labs, _ = file_to_windows(f, label=1)
    train_features += feats
    train_labels += labs

X_train = pd.DataFrame(train_features).values.astype(np.float32)
y_train = np.array(train_labels, dtype=np.float32)

# 정규화
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)

pickle.dump(scaler, open("model/scaler.pkl", "wb"))
np.save("model/scaler_mean.npy", scaler.mean_)
np.save("model/scaler_std.npy", scaler.scale_)

# 모델
model = keras.Sequential([
    keras.layers.Input(shape=(X_train.shape[1],)),
    keras.layers.Dense(64, activation='relu'),
    keras.layers.Dropout(0.3),
    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dropout(0.2),
    keras.layers.Dense(16, activation='relu'),
    keras.layers.Dense(1, activation='sigmoid'),
])

model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)

model.fit(
    X_train, y_train,
    epochs=100,
    batch_size=32,
    callbacks=[keras.callbacks.EarlyStopping(
        patience=10,
        restore_best_weights=True
    )]
)

model.save("model/motor_model.h5")
print("✅ 학습 완료 및 모델 저장")
