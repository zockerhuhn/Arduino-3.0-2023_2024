void stop()
{
  motors.setSpeeds(0, 0);
}

void straight(float factor = 1) //drive straight
{
  if (digitalRead(calibrationPin)) {
    return;
  }
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);
  motors.setSpeeds((int)(42 * factor),(int)(50 * factor)); //prevent motor drifting
}

void left(int turnBy=0) //turn left
{
  if (digitalRead(calibrationPin)) {
    return;
  }
  ReadDirection();
  int initialDirection = direction;
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  motors.setSpeeds(70, 75);
  if (turnBy!=0) {
    while ((((initialDirection - turnBy) + 360) % 360) != direction) {
      delay(10);
      ReadDirection();
    }
    stop();
  }
}

void right(int turnBy=0) //turn right
{
  if (digitalRead(calibrationPin)) {
    return;
  }
  ReadDirection();
  int initialDirection = direction;
  motors.flipLeftMotor(false);
  motors.flipRightMotor(false);
  motors.setSpeeds(70, 75);
  if (turnBy != 0) {
    while (((initialDirection + turnBy) % 360) != direction) {
      delay(10);
      ReadDirection();
    }
    stop();
  }
}

void straight_left() //drive straight but pull left
{
  if (digitalRead(calibrationPin)) {
    return;
  }
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);
  motors.setSpeeds(30, 80);
}

void straight_right() //drive straight but pull right
{
  if (digitalRead(calibrationPin)) {
    return;
  }
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);
  motors.setSpeeds(80, 30);
}


void left_to_line() {
  // going left until it finds a line  
  motors.flipLeftMotor(true);
  motors.flipRightMotor(true);
  motors.setSpeeds(35, 37.5); // probably accounting for motor deficiencies
  while (calculateReflection() != "normalLine") {
    if (calculateReflection() == "leftLine") {
      straight_left();
      break;
    } else if (calculateReflection() == "rightLine") {
      straight_right();
      break;
    }
  }
  // reset motor flips
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);

}

void right_to_line() {
  // going right until it finds a line  
  motors.flipLeftMotor(false);
  motors.flipRightMotor(false);
  motors.setSpeeds(35, 37.5);
  while (calculateReflection() != "normalLine") {
    if (calculateReflection() == "leftLine") {
      straight_left();
      break;
    } else if (calculateReflection() == "rightLine") {
      straight_right();
      break;
    }
  }
  // reset motor flips
  motors.flipLeftMotor(false);
  motors.flipRightMotor(true);
}