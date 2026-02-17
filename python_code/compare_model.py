import pandas as pd
import numpy as np

# ==========================
# 1. 결과 파일 로드
# ==========================
h5_df = pd.read_csv("result/normal_20260217_181258_result.csv")
tflite_df = pd.read_csv("result/normal_20260217_181258_result_tflite.csv")

# window 정렬 (안전)
h5_df = h5_df.sort_values("window_start").reset_index(drop=True)
tflite_df = tflite_df.sort_values("window_start").reset_index(drop=True)

# ==========================
# 2. anomaly score 비교
# ==========================
score_diff = np.abs(
    h5_df["anomaly_score"] - tflite_df["anomaly_score"]
)

print("평균 score 차이:", score_diff.mean())
print("최대 score 차이:", score_diff.max())

# ==========================
# 3. label 비교
# ==========================
label_diff = (
    h5_df["pred_label"] != tflite_df["pred_label"]
).sum()

print("label 다른 window 개수:", label_diff)

# ==========================
# 4. 비교 CSV 저장 (추천)
# ==========================
compare_df = pd.DataFrame({
    "window_start": h5_df["window_start"],
    "h5_score": h5_df["anomaly_score"],
    "tflite_score": tflite_df["anomaly_score"],
    "score_diff": score_diff,
    "h5_label": h5_df["pred_label"],
    "tflite_label": tflite_df["pred_label"]
})

compare_df.to_csv("result/compare_result.csv", index=False)
print("비교 결과 저장 완료")
