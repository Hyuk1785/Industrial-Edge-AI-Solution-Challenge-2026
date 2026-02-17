import pandas as pd
import numpy as np
import glob
import tensorflow as tf
from tensorflow import keras
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, confusion_matrix
import pickle

# ── 1. 파일별로 따로 로드 ──
normal_files = sorted(glob.glob('normal_*.csv'))
defect_files = sorted(glob.glob('defect_*.csv'))

print(f"Normal 파일: {normal_files}")
print(f"Defect 파일: {defect_files}")

# ── 2. Feature 추출 함수 ──
WINDOW = 50
STEP   = 25

def extract_features(window):
    ax  = window['ax'].values
    ay  = window['ay'].values
    az  = window['az'].values
    gx  = window['gx'].values
    gy  = window['gy'].values
    gz  = window['gz'].values
    cur = window['current'].values
    vol = window['voltage'].values
    pwr = window['power'].values
    accel_mag = np.sqrt(ax**2 + ay**2 + az**2)

    return {
        'ax_std':           ax.std(),
        'ay_std':           ay.std(),
        'az_std':           az.std(),
        'ax_p2p':           ax.max() - ax.min(),
        'ay_p2p':           ay.max() - ay.min(),
        'az_p2p':           az.max() - az.min(),
        'accel_mag_std':    accel_mag.std(),
        'accel_mag_mean':   accel_mag.mean(),
        'gx_std':           gx.std(),
        'gy_std':           gy.std(),
        'gz_std':           gz.std(),
        'gx_p2p':           gx.max() - gx.min(),
        'gy_p2p':           gy.max() - gy.min(),
        'gz_p2p':           gz.max() - gz.min(),
        'current_mean':     cur.mean(),
        'current_std':      cur.std(),
        'current_p2p':      cur.max() - cur.min(),
        'voltage_mean':     vol.mean(),
        'voltage_std':      vol.std(),
        'power_mean':       pwr.mean(),
        'current_over_019': (cur >= 0.19).sum() / len(cur),
    }

def file_to_windows(filepath, label):
    """파일 하나 → 윈도우 리스트 (겹침 없이 순서대로)"""
    df = pd.read_csv(filepath)
    features, labels = [], []
    for start in range(0, len(df) - WINDOW, STEP):
        window = df.iloc[start:start + WINDOW]
        features.append(extract_features(window))
        labels.append(label)
    return features, labels

# ── 3. 파일 단위로 Train / Test 분할 ──
# normal: 2개 → 1개는 train, 1개는 test
# defect: 2개 → 1개는 train, 1개는 test

train_features, train_labels = [], []
test_features,  test_labels  = [], []

for i, f in enumerate(normal_files):
    feats, labs = file_to_windows(f, label=0)
    if i == 0:  # 첫번째 파일 → train
        train_features += feats
        train_labels   += labs
        print(f"[TRAIN] {f} → {len(feats)}개 윈도우")
    else:        # 나머지 → test
        test_features += feats
        test_labels   += labs
        print(f"[TEST]  {f} → {len(feats)}개 윈도우")

for i, f in enumerate(defect_files):
    feats, labs = file_to_windows(f, label=1)
    if i == 0:
        train_features += feats
        train_labels   += labs
        print(f"[TRAIN] {f} → {len(feats)}개 윈도우")
    else:
        test_features += feats
        test_labels   += labs
        print(f"[TEST]  {f} → {len(feats)}개 윈도우")

X_train = pd.DataFrame(train_features).values.astype(np.float32)
y_train = np.array(train_labels, dtype=np.float32)
X_test  = pd.DataFrame(test_features).values.astype(np.float32)
y_test  = np.array(test_labels, dtype=np.float32)

print(f"\nTrain: {len(X_train)}개 (정상 {(y_train==0).sum()}, 비정상 {(y_train==1).sum()})")
print(f"Test:  {len(X_test)}개  (정상 {(y_test==0).sum()},  비정상 {(y_test==1).sum()})")

# ── 4. 정규화 (train 기준으로만 fit!) ──
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)  # train으로만 fit
X_test  = scaler.transform(X_test)       # test는 transform만!

pickle.dump(scaler, open('scaler.pkl', 'wb'))
np.save('scaler_mean.npy', scaler.mean_)
np.save('scaler_std.npy',  scaler.scale_)
print("Scaler 저장 완료")

# ── 5. 모델 정의 ──
model = keras.Sequential([
    keras.layers.Input(shape=(X_train.shape[1],)),
    keras.layers.Dense(64, activation='relu'),
    keras.layers.Dropout(0.3),
    keras.layers.Dense(32, activation='relu'),
    keras.layers.Dropout(0.2),
    keras.layers.Dense(16, activation='relu'),
    keras.layers.Dense(1,  activation='sigmoid'),
])

model.compile(
    optimizer='adam',
    loss='binary_crossentropy',
    metrics=['accuracy']
)
model.summary()

# ── 6. 학습 (validation은 test셋으로 명시적 지정) ──
history = model.fit(
    X_train, y_train,
    epochs=100,
    batch_size=32,
    validation_data=(X_test, y_test),  # 명시적으로 분리된 파일 사용!
    callbacks=[
        keras.callbacks.EarlyStopping(patience=10, restore_best_weights=True),
        keras.callbacks.ReduceLROnPlateau(factor=0.5, patience=5)
    ],
    verbose=1
)

# ── 7. 평가 ──
loss, acc = model.evaluate(X_test, y_test, verbose=0)
# 예측 score (0~1 확률)
y_score = model.predict(X_test).flatten()

# 최종 예측 label
y_pred = (y_score > 0.5).astype(int)

print(f"\n테스트 정확도: {acc:.4f}")

# ===== 🔥 추가: 추론 결과 CSV 저장 =====
result_df = pd.DataFrame({
    "true_label": y_test,
    "pred_label": y_pred,
    "anomaly_score": y_score
})

result_df.to_csv("inference_result.csv", index=False)
print("추론 결과 CSV 저장 완료 → inference_result.csv")



print("\nClassification Report:")
print(classification_report(y_test, y_pred, target_names=['Normal','Defect']))
cm = confusion_matrix(y_test, y_pred)
print(f"정상→정상:         {cm[0][0]}")
print(f"정상→비정상(오탐):  {cm[0][1]}")
print(f"비정상→정상(미탐):  {cm[1][0]}")
print(f"비정상→비정상:     {cm[1][1]}")

# ── 8. TFLite 변환 ──
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open('motor_anomaly.tflite', 'wb') as f:
    f.write(tflite_model)

print(f"\nTFLite 저장 완료: motor_anomaly.tflite")
print(f"모델 크기: {len(tflite_model)/1024:.1f} KB")