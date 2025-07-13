////////////////////////////////////////////////////////////////////////
// Arduino Bluetooth Interface with Mindwave
//
// This is example code provided by NeuroSky, Inc. and is provided
// license free.
////////////////////////////////////////////////////////////////////////

// Modified by Paulo Loma Marconi 30/08/2015

#define qualityLed  19
#define BAUDRATE    57600
#define DEBUGOUTPUT 0

//#define powercontrol 10

//---- variables y pines bot
int enA_M1 = 3, in1_M1 = 2,  in2_M1 = 4;
int enB_M2 = 5, in3_M2 = 7,  in4_M2 = 14;

// checksum variables
byte generatedChecksum = 0;
byte checksum = 0;
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
byte attention = 0;
byte meditation = 0;
 
// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;

//////////////////////////
// Microprocessor Setup //
//////////////////////////
void setup()
{
  pinMode(qualityLed, OUTPUT);

  pinMode(enA_M1, OUTPUT);pinMode(in1_M1, OUTPUT);pinMode(in2_M1, OUTPUT);
  pinMode(enB_M2, OUTPUT);pinMode(in3_M2, OUTPUT);pinMode(in4_M2, OUTPUT);

  Serial.begin(BAUDRATE);           // USB
}

////////////////////////////////
// Read data from Serial UART //
////////////////////////////////

byte ReadOneByte()
{
  int ByteRead;
  while(!Serial.available());
  ByteRead = Serial.read();
 
  #if DEBUGOUTPUT 
    Serial.print((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
  #endif

  return ByteRead;
}

/////////////
//MAIN LOOP//
/////////////
void loop()
{
  // Look for sync bytes
  if(ReadOneByte() == 170)
  {
    if(ReadOneByte() == 170)
    {
      payloadLength = ReadOneByte();
      
      if(payloadLength > 169)          //Payload length can not be greater than 169
	return;
      
      generatedChecksum = 0;       
      
      for(int i = 0; i < payloadLength; i++)
      { 
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }  
        
      checksum = ReadOneByte();         //Read checksum byte from stream     
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

      if(checksum == generatedChecksum)
      {   
        poorQuality = 200;
        attention = 0;
        meditation = 0;

        for(int i = 0; i < payloadLength; i++)
        {                                          // Parse the payload
          switch (payloadData[i])
          {
            case 2:
              i++;           
              poorQuality = payloadData[i];
              bigPacket = true;           
              break;
            case 4:
              i++;
              attention = payloadData[i];                       
              break;
            case 5:
              i++;
              meditation = payloadData[i];
              break;
            case 0x80:
              i = i + 3;
              break;
            case 0x83:
              i = i + 25;     
              break;
            default:
              break;
            } // switch
        } // for loop

      #if !DEBUGOUTPUT
        // *** Add your code here ***
        if(bigPacket)
        {
          if(poorQuality == 0)
          {
	    digitalWrite(qualityLed, HIGH);
            int speed = 255;
            if(attention >= 60 && meditation < 60)
            {  // forward
              analogWrite(enA_M1, speed); digitalWrite(in1_M1, LOW); digitalWrite(in2_M1, HIGH);
              analogWrite(enB_M2, speed); digitalWrite(in3_M2, HIGH); digitalWrite(in4_M2, LOW);              
            }
            else if(meditation >= 60 && attention < 60)
            {  // backward
              analogWrite(enA_M1, speed); digitalWrite(in1_M1, HIGH); digitalWrite(in2_M1, LOW);
              analogWrite(enB_M2, speed); digitalWrite(in3_M2, LOW); digitalWrite(in4_M2, HIGH);              
            } 
            /*else if(attention >= 40 && attention < 80)
            {  // right
              analogWrite(enA_M1, speed); digitalWrite(in1_M1, HIGH); digitalWrite(in2_M1, LOW);
              analogWrite(enB_M2, speed); digitalWrite(in3_M2, HIGH); digitalWrite(in4_M2, LOW);              
            }
            else if(meditation >= 40 && meditation < 80)
            {  // left
              analogWrite(enA_M1, speed); digitalWrite(in1_M1, LOW); digitalWrite(in2_M1, HIGH);
              analogWrite(enB_M2, speed); digitalWrite(in3_M2, LOW); digitalWrite(in4_M2, HIGH);                          
            }*/
            else if(attention < 60 || meditation <60)
            { // stop
              analogWrite(enA_M1, 0); //digitalWrite(in1_M1, LOW); digitalWrite(in2_M1, HIGH);
              analogWrite(enB_M2, 0); //digitalWrite(in3_M2, LOW); digitalWrite(in4_M2, HIGH);              
            }                             
          }
          else
            digitalWrite(qualityLed, LOW);
			
          Serial.print("PoorQuality: ");
          Serial.print(poorQuality, DEC);
          Serial.print(" Attention: ");
          Serial.print(attention, DEC);
          Serial.print(" Meditation: ");
          Serial.print(meditation, DEC);
          Serial.print(" Time since last packet: ");
          Serial.print(millis() - lastReceivedPacket, DEC);
          lastReceivedPacket = millis();
          Serial.print("\n");
                    
        }
     #endif       
        bigPacket = false;       
      }
      else {
        // Checksum Error
      }  // end if else for checksum
    } // end if read 0xAA byte
  } // end if read 0xAA byte
}
