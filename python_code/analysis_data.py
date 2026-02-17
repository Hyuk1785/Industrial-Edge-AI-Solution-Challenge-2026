import pandas as pd
import matplotlib.pyplot as plt

NORMAL_CSV = "result/normal_result.csv"
DEFECT_CSV = "result/defect_result.csv"

# 결과 CSV 읽기
normal = pd.read_csv(NORMAL_CSV)
defect = pd.read_csv(DEFECT_CSV)

plt.figure(figsize=(12,5))

plt.plot(normal["window_start"],
         normal["anomaly_score"],
         label="Normal")

plt.plot(defect["window_start"],
         defect["anomaly_score"],
         label="Defect")

plt.axhline(0.5, linestyle="--")

plt.legend()
plt.grid(True)
plt.show()