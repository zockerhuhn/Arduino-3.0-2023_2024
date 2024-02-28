void readColor() //liest Farbsensor 1 aus und printed Ergebnis
{
  rgbSensor.getRawData(&rot, &gruen, &blau, &helligkeit);
  Serial.println("R:" + String(rot) + " G:" + String(gruen) + " B:" + String(blau) + " C:" + String(helligkeit));
  /** Dieser Mechanismus hier ist gefährlich, wenn es passieren kann, dass die Sensoren lange Zeit das selbe sehen:
   *  In meinen Versuchen habe ich oben den Gain von 4x auf 16x gestellt, um mehr Rauschen zu bekommen.
   *  Mit Timeout 5s sehe ich keine False-Negatives mehr: */
}

void readColor2() //liest Farbsensor 2 aus und printed Ergebnis
{
  rgbSensor2.getRawData(&rot2, &gruen2, &blau2, &helligkeit2);
  Serial.println("R:" + String(rot2) + " G:" + String(gruen2) + " B:" + String(blau2) + " C:" + String(helligkeit2));
  /** Dieser Mechanismus hier ist gefährlich, wenn es passieren kann, dass die Sensoren lange Zeit das selbe sehen:
   *  In meinen Versuchen habe ich oben den Gain von 4x auf 16x gestellt, um mehr Rauschen zu bekommen.
   *  Mit Timeout 5s sehe ich keine False-Negatives mehr: */
}

boolean calculateColor() //nimmt letzten ausgelesen Wert von Farbsensor 1 und return ob grün erkannt wurde als bool
{
  {
    if ((gruen-150) >= blau && (gruen-150) >= rot && helligkeit <= colorBrightMaxThreshold && helligkeit >= colorBrightMinThreshold)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

boolean calculateColor2() //nimmt letzten ausgelesen Wert von Farbsensor 2 und returnt ob grün erkannt wurde als bool
{
  {
    if ((gruen2-150) >= blau2 && (gruen2-150) >= rot2 && helligkeit2 <= colorBrightMaxThreshold && helligkeit2 >= colorBrightMinThreshold)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}
