
// inslude the SPI library:
#include <SPI.h>
// set pin 10 as the slave select for the encoder:
const int CSn = 7;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  pinMode (CSn, OUTPUT);
  /* set up spi register manually
  SPIE = 0; // interrupt
  SPE =  1; // spi enable
  DORD = 0; // msb (or lsb=0)
  MSTR = 1; // master (or slave = 0)
  CPOL = 0; // clock idle when low
  CPHA = 1; // samples on rising edge
  SPR1 = 1; // speed 
  SPR0 = 1; // speed
  SPCR_register = 0b01010111; */
  SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA) | (1<<SPR1) | (1<<SPR0);
  
}

boolean calc_even_parity(word inbyte) {
  word paritymask = 0b1000000000000000;
  word datamask   = 0b0011111111111111;
  word data = inbyte | datamask;
  int x = 13;
  boolean calculated_parity = 0;
  while(x)
    calculated_parity = calculated_parity ^ ( (data << (13-x)) >> x--); // this is stunningly opaque.
  return calculated_parity;
}
boolean check_parity (word inbyte) {    // not currently using this routine
  word parity_bit_value = inbyte & 0b1000000000000000;
  word data_value = inbyte | 0b0011111111111111;
  if (calc_even_parity(data_value) == parity_bit_value)
    return 0;
  else
    return 1;
}

long readRegister(word thisRegister, int bytesToRead ) {
  byte inByte = 0;   // incoming byte from the SPI
  long result = 0;   // result to return
  byte lowbyte = thisRegister & 0b0000000011111111;
  byte highbyte = (thisRegister >> 8);
  digitalWrite(CSn, LOW);
  SPI.transfer(highbyte);
  result = SPI.transfer(lowbyte);
  digitalWrite(CSn, HIGH);
  delayMicroseconds(10);
  digitalWrite(CSn, LOW);
  bytesToRead--;
  while (bytesToRead-- > 0) {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    inByte = SPI.transfer(0x00);
    // combine the byte with the previous one:
    result = result | inByte;
  }
  // take the chip select high to de-select:
  digitalWrite(CSn, HIGH);
  // return the result:
  return(result);
}

void loop() {
  
  word data_register = 0x3FFF;
  word uncor_data_register = 0x3FFE;
  word err_register = 0x0001;
  word diag_register = 0x3FFC;
  word result;
    
  word errfl_p = 0x4001;
  word angleunc_p = 0x7FFE;
  word anglecom_p = 0xFFFF;
  word diaagc_p = 0xFFFC;
  word mask_results = 0b0011111111111111; // this grabs the returned data without the read/write or parity bits.
  
  while (1)
  {
    result = readRegister(anglecom_p, 3); // I'm precomputing the parity
    result = result & mask_results; //   and masking out the resulting parity and read/write bits
    Serial.print("received data register: ");
    Serial.println(result, DEC);
    result = 0;
    delay(1000);
    result = readRegister(angleunc_p, 3);
    result = result & mask_results;
    Serial.print("received uncor data register: ");
    Serial.println(result, DEC);
    result = 0;
    delay(1000);
    result = readRegister(errfl_p, 3);
    result = result & mask_results;
    Serial.print("received err register: ");
    Serial.println(result, DEC);
    result = 0;
    delay(1000);
    result = readRegister(diaagc_p, 3);
    result = result & mask_results;
    Serial.print("received diag/agc register: ");
    Serial.println(result, DEC);
    result = 0;
    delay(1000);
  }
}

