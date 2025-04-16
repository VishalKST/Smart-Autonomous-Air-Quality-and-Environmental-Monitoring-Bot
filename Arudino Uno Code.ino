#define CH1 2   // FS-R6B CH1 (Left Motor Forward)
#define CH2 3   // FS-R6B CH2 (Left Motor Reverse)
#define CH3 4   // FS-R6B CH3 (Right Motor Forward)
#define CH4 5   // FS-R6B CH4 (Right Motor Reverse)

#define IN1 6   // L298N IN1
#define IN2 7   // L298N IN2
#define IN3 8   // L298N IN3
#define IN4 9   // L298N IN4

void setup() {
    pinMode(CH1, INPUT);
    pinMode(CH2, INPUT);
    pinMode(CH3, INPUT);
    pinMode(CH4, INPUT);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    Serial.begin(9600);
}

void loop() {
    int ch1_value = pulseIn(CH1, HIGH, 25000);  // Read PWM signal
    int ch2_value = pulseIn(CH2, HIGH, 25000);
    int ch3_value = pulseIn(CH3, HIGH, 25000);
    int ch4_value = pulseIn(CH4, HIGH, 25000);

    Serial.print("CH1: "); Serial.print(ch1_value);
    Serial.print(" | CH2: "); Serial.print(ch2_value);
    Serial.print(" | CH3: "); Serial.print(ch3_value);
    Serial.print(" | CH4: "); Serial.println(ch4_value);

    // Convert PWM values to HIGH/LOW signals for L298N
    digitalWrite(IN1, ch1_value > 1400 ? HIGH : LOW);
    digitalWrite(IN2, ch2_value > 1400 ? HIGH : LOW);
    digitalWrite(IN3, ch3_value > 1400 ? HIGH : LOW);
    digitalWrite(IN4, ch4_value > 1400 ? HIGH : LOW);
}
