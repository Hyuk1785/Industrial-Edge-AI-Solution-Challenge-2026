# export_tflite.py
import tensorflow as tf
from tensorflow import keras

model = keras.models.load_model("model/motor_model.h5")

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]

tflite_model = converter.convert()

with open("model/motor_anomaly.tflite", "wb") as f:
    f.write(tflite_model)

print("✅ TFLite 변환 완료")
print(f"모델 크기: {len(tflite_model)/1024:.1f} KB")
