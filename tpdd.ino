/*
  Tandy Portable Disk Drive emulator
 */
#include <SD.h>
#include <SoftwareSerial.h>

//SoftwareSerial mySerial(6, 7); // RX, TX

//#define DEBUG_PRINT(x) Serial.print(x)
//#define DEBUG_PRINT1(x,y) Serial.print(x,y)
//#define DEBUG_PRINTLN(x) Serial.println(x)
//#define DEBUG_PRINTLN1(x,y) Serial.println(x,y)

#define DEBUG_PRINT(x) 
#define DEBUG_PRINT1(x,y)
#define DEBUG_PRINTLN(x) 
#define DEBUG_PRINTLN1(x,y) 

char preamble[2];
char data[255];
int bufpos;
int command_type;
int length;
int checksum;

int state;
#define initial_state 0
#define start_preamble 1
#define got_preamble 2
#define got_command_type 3
#define getting_data 4
#define processing_command 5

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);
  if (!SD.begin(8)) 
  {
    Serial.println("Card failed, or not present");
  } 
  else {
    Serial.println("card initialized.");
    printDirectory();
    //listFile();
  }

  
  // set the data rate for the SoftwareSerial port
 // mySerial.begin(19200);
 // bufpos = 0;

  // Clear the trash out  
 // while(mySerial.available()) {
 //     mySerial.read();
 // }
  
  state = initial_state;
}

void loop() {
  // If there's data available
  //if (mySerial.available()) {
    if (false) {
    
    switch(state) {
      case initial_state:
        //preamble[0] = mySerial.read();
        DEBUG_PRINTLN("start preamble");
        state = start_preamble;
        break;
        
      case start_preamble:
        //preamble[1] = mySerial.read();
        DEBUG_PRINT("got preamble:");
        DEBUG_PRINT(preamble[0]);
        DEBUG_PRINTLN(preamble[1]);
        state = got_preamble;
        break;
      
      case got_preamble:
        if (preamble[0] != 'Z' or preamble[1] != 'Z') {
          state=initial_state;  // Ignore the input
          DEBUG_PRINT("BAD preamble:");
          DEBUG_PRINT(preamble[0]);
          DEBUG_PRINTLN(preamble[1]);
        }
        else {
          //command_type = mySerial.read();
          state=got_command_type;
          DEBUG_PRINT("got command type:");
          DEBUG_PRINTLN1(command_type,HEX);
        }
        break;
        
      case got_command_type:
        //length = mySerial.read();
        state = getting_data;
        DEBUG_PRINT("got length:");
        DEBUG_PRINTLN1(command_type,HEX);
        bufpos=0;
        break;
      
      case getting_data:
        if (bufpos >= length) {
          //checksum = mySerial.read();
          state = processing_command;
          DEBUG_PRINT("got data - Checksum:");
          DEBUG_PRINTLN1(checksum,HEX);
        }
        else {
          //data[bufpos++] = mySerial.read();
          DEBUG_PRINT("Got byte ");
          DEBUG_PRINT(bufpos);
          DEBUG_PRINT(" of ");
          DEBUG_PRINT(length);
          DEBUG_PRINT(":");
          DEBUG_PRINTLN1(data[bufpos-1],HEX);
        }
        break;
                
    } // switch
  }
  
    if (state == processing_command) {
        DEBUG_PRINT("Processing command type = ");
        DEBUG_PRINT1(command_type,HEX);
        DEBUG_PRINT(" length = ");
        DEBUG_PRINT1(length,HEX);
        DEBUG_PRINT(" csum = ");
        DEBUG_PRINTLN1(checksum,HEX);
    
        if (calc_sum() != checksum) {
          DEBUG_PRINT("Bad checksum: ");
          DEBUG_PRINTLN1(calc_sum(),HEX);
          dump_data();
          normal_return(0x36);
          state=initial_state;
        }
        
//        switch(command_type) {
//          case 0x00:  /* Directory ref */
//            process_directory_command();
//            break;
//          case 0x01:  /* Open file */
//            open_file(data[0]);
//            break;
//          case 0x02:  /* Close file */
//            close_file();
//            break;
//          case 0x03:  /* Read */
//            read_file();
//            break;
//          case 0x04:  /* Write */
//            write_file();
//            break;
//          case 0x05:  /* Delete */
//            Serial.println("Processing delete file");
//            //unlink ((char *) filename);
//            normal_return(0x00);
//            break;
//          case 0x06:  /* Format disk */
//            Serial.println("Ignoring format disk command");
//            normal_return(0x00);
//            break;
//          case 0x07:  /* Drive Status */
//            Serial.println("Ignoring drive status command");
//            normal_return(0x00);
//            break;
//          case 0x08:  /* Drive condition */
//            respond_place_path();
//            break;
//          case 0x0D:  /* Rename File */
//            rename_file();
//            break;
//          case 0x23:  /* TS-DOS mystery command 2 */
//            respond_mystery2();
//            break;
//          case 0x31:  /* TS-DOS mystery command 1 */
//            //respond_mystery();
//            DEBUG_PRINTLN("Processing mystery command 1");
//            break;
//        default:
//            DEBUG_PRINT("Unknown command type:");
//            DEBUG_PRINTLN1(command_type,HEX);
//            break;
//    } // Command type switch
    state=initial_state;
  }
}

void dump_data() {
  for(int i = 0; i < bufpos; i++) {
    DEBUG_PRINT1(data[i],HEX);
    DEBUG_PRINT(",");
  }
  DEBUG_PRINTLN("");
}

void process_directory_command() {
  Serial.print("Processing directory ref: ");

  int search_form = data[length-1];
  Serial.println(search_form,HEX);
  
  switch (search_form)
  {
    case 0x00:  /* Pick file for open/delete */
      Serial.println("Pick file for open/delete");
      break;

    case 0x01:  /* "first" directory block */
        Serial.println("Get first directory block");
        break;

    case 0x02:  /* "next" directory block */
        Serial.println("Get next directory block");
        break;

//    case 0x03:  /* "previous" directory block */
//        Serial.println("Get previous directory block");
//        break;

//    case 0x04:  /* end directory reference */
//        Serial.println("End directory reference");
//        break;
        
    default:
        Serial.print("Unknown directory command:");
        Serial.println(search_form,HEX);
        break;
    }
}

void open_file(int omode)
{
  Serial.print("Processing open file:");
  Serial.println(omode,HEX);
  
  switch(omode) {
    case 0x01:  /* New file for my_write */
      Serial.println("New file for write");
      break;
      
    case 0x02:  /* existing file for append */
      Serial.println("Existing for append");
      break;
      
    case 0x03:  /* Existing file for read */
      Serial.println("Existing for read");
        break;
    }
}

void close_file() {
  Serial.println("Processing close file");
  normal_return(0x00);
}

void read_file() {
  Serial.println("Processing read file");
}

void write_file() {
  Serial.println("Processing write file");
}

void delete_file() {
  Serial.println("Processing delete file");
}

void rename_file() {
  Serial.println("Processing file rename command");
}

void respond_place_path()
{
  static unsigned char canned[] =
    {0x12, 0x0b, 0x00, 0x52, 0x4f, 0x4f, 0x54, 0x20, 0x20, 0x2e, 0x3c, 0x3e, 0x20, 0x96};

  Serial.println("Processing drive condition command");

  send_data(canned,sizeof(canned));
}

void respond_mystery()
{
  static unsigned char canned[] = {0x38, 0x01, 0x00, 0xC6};

  send_data(canned,sizeof(canned));
}

void respond_mystery2()
{
  static unsigned char canned[] =
      {0x14, 0x0F, 0x41, 0x10, 0x01,
       0x00, 0x50, 0x05, 0x00, 0x02,
       0x00, 0x28, 0x00, 0xE1, 0x00,
       0x00, 0x00, 0x2A};

  Serial.println("Processing mystery command 2");

  send_data(canned,sizeof(canned));
}

void send_data(unsigned char data[], int length) {
  for(int i=0; i < length; i++) {
    //mySerial.write(data[i]);
  }
}

void listFile() {
  File dataFile = SD.open("ADVEN1.DO");
  
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening file");
  }
}

void printDirectory() {
  File root;
  root = SD.open("/");
  
   while(true) {
     
     File entry =  root.openNextFile();
     if (! entry) {
       // no more files
       Serial.println("**nomorefiles**");
       break;
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

void normal_return(unsigned char type)
{
  command_type = 0x12;
  length = 0x01;
  data[0] = type;
  checksum = calc_sum();
  
  //mySerial.write(command_type);
  //mySerial.write(length);
  //mySerial.write(type);
  //mySerial.write(checksum);

  DEBUG_PRINT("Response: ");
  DEBUG_PRINTLN1(type,HEX);
}

int calc_sum()
{
  unsigned short sum=0;
  int i;
  
  sum+=command_type;
  sum+=length;
  for(i=0;i<length;i++)
      sum+=data[i];
  return((sum & 0xFF) ^ 255);
}

