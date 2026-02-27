# 전류·진동 융합 기반 온디바이스 AI 모터 이상 탐지 시스템(MADS)

전류(INA219) + 진동(MPU6050) 센서 데이터를 기반으로 모터의 정상/이상 상태를 학습하고, TinyML(TFLite Micro) 형태로 임베디드(STM32)에서 실시간 추론하여 제어 로직까지 연결한 프로젝트입니다.

![project_image1](./image/project_image1.PNG)
![project_image2](./image/project_image2.PNG)

1) 프로젝트 시연 영상(전체 동작 과정)

- 전류·진동 데이터를 수집하고, 온디바이스 AI로 이상을 판정한 뒤 결과가 알림/로그로 반영되는 전체 시나리오 흐름을 보여줍니다.
- https://www.youtube.com/watch?v=rmL8mo_q7-o

2) 프로젝트 발표 영상(내부 구현 상세)

- 데이터 처리/학습/모델 변환/MCU 추론/판정 로직 등 시스템의 구현 구조와 핵심 설계 포인트를 설명합니다.
- https://www.youtube.com/watch?v=ldMNoJ08gf0

---

## 1) 프로젝트 개요

본 프로젝트는 모터에 발생하는 과부하/이상 상태를 전류 및 진동 신호로 감지하고, Python 기반으로 학습한 모델을 TFLite로 변환하여 STM32CubeIDE 환경의 C 코드에서 추론 결과를 활용해 LED/부저/모터 제어 등의 로직을 수행하도록 설계했습니다.

### 입력 데이터
- 전류/전압/전력(INA219)
- 가속도/자이로(MPU6050)

### 학습 방식
- 정상 + 이상 데이터 기반 Supervised 학습(분류)
- 또는 이상탐지 기반 구조 (프로젝트 설계 방향에 따라)

### 배포/추론
- TensorFlow/Keras → TFLite 변환 → STM32(C)에서 추론 및 제어

---

## 2) 핵심 기능
- 전류 + 진동 센서 기반 데이터 수집 및 CSV 로깅
- 정상/이상 데이터셋 구성 및 전처리(정규화, 윈도우링 등)
- TensorFlow/Keras 모델 학습
- TFLite 변환 및 Python에서 TFLite 추론 검증
- STM32CubeIDE(C 코드)에서 센서 값 수신 → 전처리 → TFLite 추론
- 추론 결과 기반 제어 로직(상태 표시/알림/제어) 설계

---

## 3) 사용 기술 (Tech Stack)

### ML / Data
- Python
- TensorFlow / Keras
- TFLite (Inference)

### Embedded
- STM32CubeIDE
- C (STM32 HAL 기반 센서 값 수집/제어 로직)
---

## 4) 시스템 아키텍처

### 센서 데이터 수집
- INA219, MPU6050 측정값을 MCU에서 읽어옴

### 데이터 전처리
- 학습/추론 동일하게 정규화, 윈도우링(슬라이딩 윈도우) 적용

### 모델 학습
- Python 환경에서 정상/이상 데이터로 Keras 모델 학습

### 모델 변환
- 학습된 모델을 TFLite로 변환

### 임베디드 추론
- STM32에서 실시간으로 입력 특징을 구성하고 TFLite 모델로 추론

### 제어 로직
- 추론 결과(정상/이상)에 따라 LED/부저/모터 제어 등 동작 수행

---

## 5) 데이터 구성 (예시)

CSV 컬럼 예시:
- time_s
- ax, ay, az (가속도)
- gx, gy, gz (자이로)
- current, voltage, power (전류/전압/전력)

학습과 추론에서 같은 전처리/특징 생성 규칙을 사용하는 것이 핵심입니다.

---

## 6) 하드웨어 구성 (Hardware)

본 프로젝트는 전류/진동 센서 데이터를 기반으로 모터 이상을 판단하고, 2개의 STM32 보드로 제어와 추론 역할을 분리하여 구성했습니다.
- 부품 목록(BOM): 하드웨어 부품목록.xlsx
- 하드웨어 핀맵(Pin Map): Hardware_PinMap/STM보드 최종본(L432KC+F103RB).xlsx

### MCU / Boards

- STM32 Nucleo-F103RB (Control MCU)
  - 모터 구동 및 제어 로직 수행(PWM, GPIO)
  - 상태 표시(LED), 알림(부저), 서보/버튼/OLED 등 주변장치 제어

- STM32 Nucleo-L432KC (Inference MCU)
  - INA219/MPU6050 센서 값 수집(I2C)
  - 특징 추출(Feature Extraction) 및 TinyML 추론(TFLite Micro)
  - 추론 결과 기반 이상 상태 판단 및 상위 제어 로직과 연동

### Sensors
- INA219: 전류/전압/전력 측정(과부하/이상 상태 특징)
- MPU6050: 가속도/자이로 측정(진동 기반 이상 징후 특징)

### Actuators / Devices
- DC Motor
- Motor Driver: (예: L298N/L298P 등)
- 3-Color LED: 상태 표시(정상/주의/이상)
- Buzzer: 이상 알림

- Timing Belt + Pulley
  - 서보모터 동작과 연계해 벨트 장력을 변화시켜 모터 축에 외력/부하를 인가(이상 상태 재현용)

- Servo Motor
  - 타이밍 벨트 장력 조절로 모터 부하를 인가하여 이상 상태를 재현(Fault Injection)

- Button: 사용자 입력(모드/동작 트리거)
- OLED(SSD1306): 상태/로그 표시

---

## 7) 프로젝트 구조 (Project Structure)

```bash
Industrial-Edge-AI-Solution-Challenge-2026/
├─ F103RB_Control_MCU/
│  └─ Core/
│     ├─ Inc/                      # 헤더파일 모음
│     └─ Src/                      # 소스파일 모음
│        ├─ button.c               # 버튼 제어
│        ├─ buzzer.c               # 부저 제어
│        ├─ led.c                  # 3색 LED 제어
│        ├─ main.c                 # 메인 동작
│        ├─ motor.c                # 모터 제어(PWM)
│        ├─ servo.c                # 서보모터 제어
│        └─ ssd1306.c              # OLED 제어(SSD1306)
│           ├─ ssd1306_conf.h
│           └─ ssd1306_fonts.h
│
├─ L432KC_Infer_MCU/
│  └─ Core/
│     ├─ Inc/                      # 헤더파일 모음
│     └─ Src/                      # 소스파일 모음
│        ├─ anomaly_detection.c    # 이상 탐지(추론)
│        ├─ feature_extraction.c   # 특징 추출
│        ├─ ina219.c               # 전류 센서(INA219) 값 수집
│        ├─ mpu6050.c              # 진동 센서(MPU6050) 값 수집
│        └─ main.c                 # 메인 동작
│
├─ ai_python/
│  ├─ train.py                     # 학습 코드
│  ├─ infer.py                     # CSV 추론 및 결과 저장
│  ├─ utils.py                     # CSV 파일 처리 유틸
│  ├─ mads_gui.py                  # GUI
│  ├─ export_tflite.py             # TFLite 변환
│  ├─ data/                        # 학습 데이터셋(normal/defect)
│  ├─ model/                       # 학습된 모델, scaler
│  └─ result/                      # 추론 결과 데이터
│
└─ README.md
