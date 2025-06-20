// designed & programmed by ypwl, find me @ https://github.com/yanpiaowanli/design-and-build-2

#include <Servo.h>

void handleSerial();
void stepPulseIfDue();
void moveAllServosTo(int target_angle, bool is_smooth = false);
void moveServoTo(int servo_index, int target_angle, bool is_smooth = true);
void handleSetCommand(String input);
void handleRunScriptCommand(String input);
void handleActionWithArgs(String arg, String action);
void logInfo(const String& msg);
void logError(const String& msg);

const int NUM_SERVOS = 4;
Servo servos[NUM_SERVOS];
const int servoPins[NUM_SERVOS] = {11, 10, 9, 6};
const int STEP_DELAY = 100;

int initAngles[NUM_SERVOS] = {23, 45, 160, 160};
int currentAngles[NUM_SERVOS] = {23, 45, 160, 160};
int d60Angles[NUM_SERVOS] = {23, 45, 160, 160};
int d80Angles[NUM_SERVOS] = {3, 5, 160, 160};
int longestAngles[NUM_SERVOS] = {83, 165, 80, 160};

const int PUL_PIN = 12;
const int DIR_PIN = 13;

const float STEPS_PER_SECOND = 5000.0;
const unsigned int PULSE_WIDTH_US = 5;
const unsigned int INTERVAL_US =
    static_cast<unsigned int>(1000000.0 / STEPS_PER_SECOND);

bool is_running = false;
bool is_facing_right = true;
unsigned long last_step_time = 0;

void setup() {
    Serial.begin(9600);
    for (int servo_index = 0; servo_index < NUM_SERVOS; servo_index++) {
        servos[servo_index].attach(servoPins[servo_index]);
        servos[servo_index].write(initAngles[servo_index]);
        currentAngles[servo_index] = initAngles[servo_index];
    }

    pinMode(PUL_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    digitalWrite(DIR_PIN, is_facing_right);
    digitalWrite(PUL_PIN, LOW);

    delay(1000);
    logInfo("控制已启动；请输入指令 rs [-s] <action> / set <id> <deg>");
}

void loop() {
    handleSerial();
    stepPulseIfDue();
}

inline void handleSerial() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.toLowerCase();
        input.trim();

        if (input.startsWith("set")) {
            handleSetCommand(input);
        } else if (input.startsWith("rs") or input.startsWith("runscript")) {
            handleRunScriptCommand(input);
        } else {
            logError("未知命令：" + input);
        }
    }
}

inline void stepPulseIfDue() {
    if (not is_running)
        return;
    unsigned long now = micros();
    if (now - last_step_time >= INTERVAL_US) {
        last_step_time = now;
        digitalWrite(PUL_PIN, HIGH);
        delayMicroseconds(PULSE_WIDTH_US);
        digitalWrite(PUL_PIN, LOW);
    }
}

void handleSetCommand(String input) {
    int space_idx_1 = input.indexOf(' ');
    int space_idx_2 = input.indexOf(' ', space_idx_1 + 1);
    bool is_legal = (space_idx_1 != -1) and (space_idx_2 != -1);
    if (not is_legal) {
        logError("格式错误，应为: set <index(0-3)> <angle(0-180)>");
        return;
    }

    int servo_index = input.substring(space_idx_1 + 1, space_idx_2).toInt();
    int target_angle = input.substring(space_idx_2 + 1).toInt();

    if (servo_index >= 0 and servo_index < NUM_SERVOS and target_angle >= 0 and
        target_angle <= 180) {
        moveServoTo(servo_index, target_angle);
        logInfo("舵机 #" + String(servo_index) + " 设置为 " +
                String(target_angle) + "°");
    } else {
        logError("舵机编号或角度无效");
    }
}

void handleRunScriptCommand(String input) {
    int space_idx_1 = input.indexOf(' ');
    bool is_legal = (space_idx_1 != -1);
    int space_idx_2 = input.indexOf(' ', space_idx_1 + 1);
    bool has_arg = (space_idx_2 != -1);

    String arg_1 = "";
    String action = "";
    if (not is_legal) {
        logError("未知 runsript 指令格式，示例: rs extend / rs -s retract");
        return;
    }
    if (has_arg) {
        arg_1 = input.substring(space_idx_1 + 1, space_idx_2);
        action = input.substring(space_idx_2 + 1);
    } else {
        action = input.substring(space_idx_1 + 1);
    }

    handleActionWithArgs(arg_1, action);
}

void handleActionWithArgs(String arg, String action) {
    // bool is_force_smooth = (arg == "-s");
    bool is_force_smooth = true;

    if (action == "extend") {
        moveServoTo(1, 165, is_force_smooth);
        moveServoTo(0, 83, is_force_smooth);
        moveServoTo(2, 105, is_force_smooth);
    } else if (action == "retract") {
        moveServoTo(0, 3, is_force_smooth);
        moveServoTo(2, 180, is_force_smooth);
        moveServoTo(1, 5, is_force_smooth);
    } else if (action == "open") {
        moveServoTo(3, 120, is_force_smooth);
    } else if (action == "close") {
        moveServoTo(3, 170, is_force_smooth);
    } else if (action == "left" or action == "l") {
        is_facing_right = false;
        digitalWrite(DIR_PIN, is_facing_right);
        is_running = true;
    } else if (action == "right" or action == "r") {
        is_facing_right = true;
        digitalWrite(DIR_PIN, is_facing_right);
        is_running = true;
    } else if (action == "stop" or action == "s") {
        is_running = false;
    } else {
        logError("未知动作: " + action + "，示例: rs extend / rs -s retract");
        return;
    }

    if (arg == "") {
        logInfo(action + " 动作完成");
    } else {
        logInfo(action + " " + arg + " 动作完成");
    }
}

void moveAllServosTo(int target_angle, bool is_smooth) {
    for (int servo_index = 0; servo_index < NUM_SERVOS; servo_index++) {
        moveServoTo(servo_index, target_angle, is_smooth);
    }
}

void moveServoTo(int servo_index, int target_angle, bool is_smooth = true) {
    if (is_smooth) {
        int current_angle = currentAngles[servo_index];
        int step = (target_angle > current_angle) ? 1 : -1;
        for (int angle = current_angle; angle != target_angle; angle += step) {
            servos[servo_index].write(angle);
            delay(STEP_DELAY);
        }
    }
    servos[servo_index].write(target_angle);
    currentAngles[servo_index] = target_angle;
}

void logInfo(const String& msg) {
    Serial.println("[INFO] " + msg);
}

void logError(const String& msg) {
    Serial.println("[ERROR] " + msg);
}
