import pandas as pd
import numpy as np
import tensorflow as tf
from tensorflow import keras
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import classification_report, confusion_matrix
import pickle

# ==================================================
# ⭐ 0. 사용자 설정 (여기만 수정하면 됨)
# ==================================================

# ---- 학습용 파일 ----
TRAIN_NORMAL_FILES = [
    "normal_20260217_180758.csv"
]

TRAIN_DEFECT_FILES = [
    "defect_20260217_182038.csv"
]

# ---- 추론용 파일 (원하는 만큼 넣기) ----
INFER_FILES = [
    #"defect_20260217_182538.csv",
    "normal_20260217_181258.csv"
]

WINDOW = 50
STEP   = 25
THRESHOLD = 0.5

# ==================================================
# 1. Feature 추출
# ==================================================

def extract_features(window):
    ax, ay, az = window['ax'], window['ay'], window['az']
    gx, gy, gz = window['gx'], window['gy'], window['gz']
    cur = window['current']
    vol = window['voltage']
    pwr = window['power']

    accel_mag = np.sqrt(ax**2 + ay**2 + az**2)

    return {
        'ax_std': ax.std(),
        'ay_std': ay.std(),
        'az_std': az.std(),
        'ax_p2p': ax.max()-ax.min(),
        'ay_p2p': ay.max()-ay.min(),
        'az_p2p': az.max()-az.min(),
        'accel_mag_std': accel_mag.std(),
        'accel_mag_mean': accel_mag.mean(),
        'gx_std': gx.std(),
        'gy_std': gy.std(),
        'gz_std': gz.std(),
        'gx_p2p': gx.max()-gx.min(),
        'gy_p2p': gy.max()-gy.min(),
        'gz_p2p': gz.max()-gz.min(),
        'current_mean': cur.mean(),
        'current_std': cur.std(),
        'current_p2p': cur.max()-cur.min(),
        'voltage_mean': vol.mean(),
        'voltage_std': vol.std(),
        'power_mean': pwr.mean(),
        'current_over_019': (cur >= 0.19).sum()/len(cur),
    }

# ==================================================
# 2. 파일 → 윈도우 변환
# ==================================================

def file_to_windows(filepath, label=None):
    df = pd.read_csv(filepath)

    features, labels, positions = [], [], []

    for start in range(0, len(df)-WINDOW, STEP):
        w = df.iloc[start:start+WINDOW]
        features.append(extract_features(w))
        positions.append(start)

        if label is not None:
            labels.append(label)

    return features, labels, positions

# ==================================================
# 3. 학습 데이터 생성
# ==================================================

train_features, train_labels = [], []

for f in TRAIN_NORMAL_FILES:
    feats, labs, _ = file_to_windows(f, label=0)
    train_features += feats
    train_labels += labs
    print(f"[TRAIN NORMAL] {f} → {len(feats)} windows")

for f in TRAIN_DEFECT_FILES:
    feats, labs, _ = file_to_windows(f, label=1)
    train_features += feats
    train_labels += labs
    print(f"[TRAIN DEFECT] {f} → {len(feats)} windows")

X_train = pd.DataFrame(train_features).values.astype(np.float32)
y_train = np.array(train_labels, dtype=np.float32)

print(f"\nTrain total: {len(X_train)}")

# ==================================================
# 4. 정규화
# ==================================================

scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)

pickle.dump(scaler, open("scaler.pkl", "wb"))
np.save("scaler_mean.npy", scaler.mean_)
np.save("scaler_std.npy", scaler.scale_)

# ==================================================
# 5. 모델
# ==================================================

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

model.summary()

# ==================================================
# 6. 학습
# ==================================================

model.fit(
    X_train, y_train,
    epochs=100,
    batch_size=32,
    callbacks=[
        keras.callbacks.EarlyStopping(patience=10,
                                      restore_best_weights=True)
    ],
    verbose=1
)

# ==================================================
# 7. 파일별 추론 (🔥 핵심)
# ==================================================

def run_inference(filepath):
    feats, _, positions = file_to_windows(filepath)

    X = pd.DataFrame(feats).values.astype(np.float32)
    X = scaler.transform(X)

    score = model.predict(X).flatten()
    pred  = (score > THRESHOLD).astype(int)

    result_df = pd.DataFrame({
        "window_start": positions,
        "anomaly_score": score,
        "pred_label": pred
    })

    save_name = filepath.replace(".csv", "_result.csv")
    result_df.to_csv(save_name, index=False)

    print(f"[INFER] {filepath} → {save_name}")

for f in INFER_FILES:
    run_inference(f)

# ==================================================
# 8. TFLite 변환
# ==================================================

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
tflite_model = converter.convert()

with open("motor_anomaly.tflite", "wb") as f:
    f.write(tflite_model)

print("\nTFLite 저장 완료")
print(f"모델 크기: {len(tflite_model)/1024:.1f} KB")
