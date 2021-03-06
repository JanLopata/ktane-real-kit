#ifdef DEBUGING
// #define DEBUGING_SR_IN
// #define DEBUGING_SR_OUT
// #define DEBUGING_SR_IN_GETVALUE
#endif


/*
  SETUP FOR INPUT/OUTPUT SHIFT REGISTERS
*/

#define SHIFT_REGISTER_INPUT_CL_EN_PIN 2
#define SHIFT_REGISTER_INPUT_PLOAD_PIN 3
#define SHIFT_REGISTER_INPUT_DATA_PIN 11
#define SHIFT_REGISTER_INPUT_CLOCK_PIN 4

#define SHIFT_REGISTER_OUTPUT_DATA_PIN 10
#define SHIFT_REGISTER_OUTPUT_LATCH_PIN 5
#define SHIFT_REGISTER_OUTPUT_CLOCK_PIN 4

#define SHIFT_REGISTER_PULSE_WIDTH 5


void setup_shift_registers() {
  // input register
  pinMode(SHIFT_REGISTER_INPUT_PLOAD_PIN, OUTPUT);
  pinMode(SHIFT_REGISTER_INPUT_CL_EN_PIN, OUTPUT);
  pinMode(SHIFT_REGISTER_INPUT_DATA_PIN, INPUT);
  pinMode(SHIFT_REGISTER_INPUT_CLOCK_PIN, OUTPUT);

  // output register
  pinMode(SHIFT_REGISTER_OUTPUT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_REGISTER_OUTPUT_LATCH_PIN, OUTPUT);
  pinMode(SHIFT_REGISTER_OUTPUT_CLOCK_PIN, OUTPUT);

  update_shift_registers();
  update_shift_registers();

}

void write_to_output_shift_register() {

  byte output_width = SRoffsetsOutput[MODULE_MAX_COUNT + 1];

#ifdef DEBUGING_SR_OUT
  Serial.print(debug_print_char);
  Serial.print(F("SR_OUT:"));
  Serial.println(output_width);
#endif

  digitalWrite(SHIFT_REGISTER_OUTPUT_LATCH_PIN, LOW);

  for (int i = output_width - 1; i >= 0; i--) {
    // sending backwards

#ifdef DEBUGING_SR_OUT
    Serial.print(debug_print_char);
    Serial.print(i);
    Serial.print(" d:");
    Serial.println(shift_register_output[i]);
#endif

    shiftOut(SHIFT_REGISTER_OUTPUT_DATA_PIN, SHIFT_REGISTER_OUTPUT_CLOCK_PIN, MSBFIRST,
             shift_register_output[i]);
  }

  digitalWrite(SHIFT_REGISTER_OUTPUT_LATCH_PIN, HIGH);

}

void read_from_input_shift_register() {
  byte input_width = SRoffsetsInput[MODULE_MAX_COUNT + 1];

#ifdef DEBUGING_SR_IN
  Serial.print(debug_print_char);
  Serial.print(F("SR_IN:"));
  Serial.println(input_width);
#endif

  digitalWrite(SHIFT_REGISTER_INPUT_CL_EN_PIN, HIGH);
  digitalWrite(SHIFT_REGISTER_INPUT_PLOAD_PIN, LOW);
  delayMicroseconds(SHIFT_REGISTER_PULSE_WIDTH);
  digitalWrite(SHIFT_REGISTER_INPUT_PLOAD_PIN, HIGH);
  digitalWrite(SHIFT_REGISTER_INPUT_CL_EN_PIN, LOW);

  for (int j = 0; j < input_width; j++) {
    byte reading = 0;
    for (int i = 0; i < 8; i++) {

      byte value = digitalRead(SHIFT_REGISTER_INPUT_DATA_PIN);

      reading |= (value << (7 - i));

      digitalWrite(SHIFT_REGISTER_INPUT_CLOCK_PIN, HIGH);
      delayMicroseconds(SHIFT_REGISTER_PULSE_WIDTH);
      digitalWrite(SHIFT_REGISTER_INPUT_CLOCK_PIN, LOW);

    }

    shift_register_input[j] = reading;
#ifdef DEBUGING_SR_IN
    Serial.print(debug_print_char);
    Serial.print(j);
    Serial.print(" r:");
    Serial.println(reading);
#endif

  }

}

void store_previous_reading() {

  byte output_width = SRoffsetsOutput[MODULE_MAX_COUNT + 1];

  for (int i = 0; i < output_width; i++) {
    shift_register_previous_input[i] = shift_register_input[i];
  }

}

void clear_previous_reading() {
  byte output_width = SRoffsetsOutput[MODULE_MAX_COUNT + 1];

  for (int i = 0; i < output_width; i++) {
    shift_register_previous_input[i] = 0;
  }

}

void update_shift_registers() {
  write_to_output_shift_register();
  store_previous_reading();
  read_from_input_shift_register();
}

void print_input_shift_register() {

  for (int i = 0; i < MODULE_MAX_COUNT; i++) {
    Serial.print(debug_print_char);
    Serial.print(i);
    Serial.print(" r:");
    Serial.println(shift_register_input[i]);
  }

}

byte get_module_input(byte module_number, byte module_mask, boolean invert) {

  int pos = SRoffsetsInput[module_number];

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("pos:");
  Serial.println(pos);
#endif

  byte reading = shift_register_input[pos];

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("r:");
  Serial.println(reading);
#endif

  if (invert) {

    reading = ~reading;
#ifdef DEBUGING_SR_IN_GETVALUE
    Serial.print(debug_print_char);
    Serial.print("inv:");
    Serial.println(reading);
#endif

  }

  reading = reading & module_mask;

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("msk:");
  Serial.println(reading);
#endif

  return reading;
}

byte get_module_previous_input(byte module_number, byte module_mask, boolean invert) {
  int pos = SRoffsetsInput[module_number];

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("ppos:");
  Serial.println(pos);
#endif

  byte reading = shift_register_previous_input[pos];

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("pr:");
  Serial.println(reading);
#endif

  if (invert) {

    reading = ~reading;
#ifdef DEBUGING_SR_IN_GETVALUE
    Serial.print(debug_print_char);
    Serial.print("pinv:");
    Serial.println(reading);
#endif

  }

  reading = reading & module_mask;

#ifdef DEBUGING_SR_IN_GETVALUE
  Serial.print(debug_print_char);
  Serial.print("pmsk:");
  Serial.println(reading);
#endif

  return reading;

}

// input signal denoising - we allow input only if it is a same as in last update
byte get_module_sanitized_input(byte module_number, byte module_mask, boolean invert) {
  byte reading = get_module_input(module_number, module_mask, invert);
  byte prev_reading = get_module_previous_input(module_number, module_mask, invert);
#ifdef DEBUGING_SR_IN
  Serial.print(debug_print_char);
  Serial.print(prev_reading);
  Serial.print(" ~ ");
  Serial.println(reading);
#endif
  if (reading == prev_reading) {
    return reading;
  } else {
    return READING_ERROR;
  }
}

