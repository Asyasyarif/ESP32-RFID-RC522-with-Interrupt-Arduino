#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         21
#define SS_PIN          5
#define IRQ_PIN         15  // Hubungkan IRQ pin RC522 ke pin ini jika ada (atau pseudo)

MFRC522 mfrc522(SS_PIN, RST_PIN);

volatile bool rfidDetected = false;

void IRAM_ATTR detectRFID() {
  rfidDetected = true;
  detachInterrupt(digitalPinToInterrupt(IRQ_PIN));
  Serial.println("TRIGGERED");
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  Serial.print(F("Ver: 0x"));
  byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.println(readReg, HEX);

  pinMode(IRQ_PIN, INPUT_PULLUP);


  // Enable interrupt untuk RX dan Idle
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg, 0xA0);  // Enable RxIRQ and IdleIRQ
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);  // Clear all IRQ flags

  attachInterrupt(digitalPinToInterrupt(IRQ_PIN), detectRFID, FALLING);



  Serial.println(F("Ready to scan RFID with interrupt..."));
}

void loop() {
 // Reset flag dulu supaya nggak dobel trigger
//  Serial.println(digitalRead(IRQ_PIN));
  if(rfidDetected){
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      rfidDetected = false;
      Serial.println("== Card Detected ==");
      Serial.print("UID: ");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();
      Serial.println("===================");

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();

      // Reset dan enable interrupt kembali
      mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F); // Clear all IRQ
      delay(50);
      attachInterrupt(digitalPinToInterrupt(IRQ_PIN), detectRFID, FALLING);

    }
  }else{
    mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
    mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
    delay(50);
  }



  // Tidak perlu tulis ulang PCD_Transceive manual
  // RC522 sudah handle polling sendiri saat idle
}