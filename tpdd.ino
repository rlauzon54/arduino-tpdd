/*
  Tandy Portable Disk Drive emulator
 */
#include <SD.h>
#include <SoftwareSerial.h>

// Hard drive activity light on pin 7
#define HDD_IND 7

#define NO_FILE 75535

// Set the serial port to be on 62,63 (A8,A9 pins)
SoftwareSerial mySerial(62, 63); // RX, TX

#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINT1(x,y) Serial.print(x,y)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLN1(x,y) Serial.println(x,y)

//#define DEBUG_PRINT(x) 
//#define DEBUG_PRINT1(x,y)
//#define DEBUG_PRINTLN(x) 
//#define DEBUG_PRINTLN1(x,y) 

char preamble[2];
unsigned char data[255];
int bufpos;
int command_type;
int length;
int checksum;
char filename[25];
File selected_file;
int selected_file_open;
unsigned long selected_file_size;

int state;
#define initial_state 0
#define start_preamble 1
#define got_preamble 2
#define got_command_type 3
#define getting_data 4
#define processing_command 5

void setup() {
  // Open serial communications for debugging
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // Set the hard drive light pin
  pinMode(HDD_IND,OUTPUT);

  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(53, OUTPUT);
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(8))
  {
    Serial.println("Card failed, or not present");
  } 
  else {
    Serial.println("card initialized.");
  }
  
  // set the data rate for the SoftwareSerial port
  mySerial.begin(19200);
  bufpos = 0;
  
  selected_file_open = 0;

  // Clear the trash out  
  while(mySerial.available()) {
      mySerial.read();
  }
  
  state = initial_state;
}

void loop() {
  // If there's data available
  if (mySerial.available()) {
  
    switch(state) {
      case initial_state:
        preamble[0] = mySerial.read();
        DEBUG_PRINTLN("start preamble");
        state = start_preamble;
        digitalWrite(HDD_IND,HIGH);
        break;
        
      case start_preamble:
        preamble[1] = mySerial.read();
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
          command_type = mySerial.read();
          state=got_command_type;
          DEBUG_PRINT("got command type:");
          DEBUG_PRINTLN1(command_type,HEX);
        }
        break;
        
      case got_command_type:
        length = mySerial.read();
        state = getting_data;
        DEBUG_PRINT("got length:");
        DEBUG_PRINTLN1(length,HEX);
        bufpos=0;
        break;
      
      case getting_data:
        if (bufpos >= length) {
          checksum = mySerial.read();
          state = processing_command;
          DEBUG_PRINT("got data - Checksum:");
          DEBUG_PRINTLN1(checksum,HEX);
        }
        else {
          data[bufpos++] = mySerial.read();
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
        
        switch(command_type) {
          case 0x00:  /* Directory ref */
            process_directory_command();
            break;
          case 0x01:  /* Open file */
            open_file(data[0]);
            break;
          case 0x02:  /* Close file */
            close_file();
            break;
          case 0x03:  /* Read */
            read_file();
            break;
          case 0x04:  /* Write */
            write_file();
            break;
          case 0x05:  /* Delete */
            Serial.println("Processing delete file");
            //unlink ((char *) filename);
            normal_return(0x00);
            break;
          case 0x06:  /* Format disk */
            Serial.println("Ignoring format disk command");
            normal_return(0x00);
            break;
          case 0x07:  /* Drive Status */
            Serial.println("Ignoring drive status command");
            normal_return(0x00);
            break;
          case 0x08:  /* Drive condition */
            respond_place_path();
            break;
          case 0x0D:  /* Rename File */
            rename_file();
            break;
          case 0x23:  /* TS-DOS mystery command 2 */
            respond_mystery2();
            break;
          case 0x31:  /* TS-DOS mystery command 1 */
            //respond_mystery();
            DEBUG_PRINTLN("Processing mystery command 1");
            break;
        default:
            DEBUG_PRINT("Unknown command type:");
            DEBUG_PRINTLN1(command_type,HEX);
            break;
    } // Command type switch
    
    state=initial_state;
    digitalWrite(HDD_IND,LOW);

  }
}

void process_directory_command() {
  Serial.print("Processing directory ref: ");

  int search_form = data[length-1];
  Serial.println(search_form,HEX);
  
  switch (search_form)
  {
    case 0x00:  /* Pick file for open/delete */
      pick_file();
      break;

    case 0x01:  /* "first" directory block */
        get_first_directory_entry();
        break;

    case 0x02:  /* "next" directory block */
        get_next_directory_entry();
        break;

    case 0x03:  /* "previous" directory block */
        get_prev_directory_entry();
        break;

    case 0x04:  /* end directory reference */
        end_directory_ref();
        break;
        
    default:
        Serial.print("Unknown directory command:");
        Serial.println(search_form,HEX);
        break;
    }
}

void pick_file() {
  Serial.println("Pick file for open/delete");
    
  // Get the file from the input
  memcpy(filename,data,24);
  filename[24]=0;
  
  // Remove trailing spaces
  for(int i=23; i > 0 && filename[i] == ' '; i--) {
    filename[i] = 0;
  }

  char *dot;
  char *p;
  /* Remove spaces between base and dot */
  dot = strchr((char *)filename,'.');
  if(dot != NULL) {
    for(p=dot-1;*p==' ';p--);
      memmove(p+1,dot,strlen((char *)dot)+1);
  }
  
  Serial.print("Picked file:");
  Serial.println(filename);
  
  // Open file
  selected_file = SD.open(filename);
  
  // If the file doesn't exist
  if (! selected_file) {
    selected_file_size = NO_FILE;  // Indicate that we didn't find the file
    selected_file_open = 0;
    directory_ref_return(NULL,0);
  }
  else { // the file exists
    selected_file_size = selected_file.size();
    selected_file_open = 1;
    directory_ref_return(filename,selected_file_size);
  }
  
}

void get_first_directory_entry() {
  Serial.println("Get first directory block");

  // If the directory is still open, close it
  if (selected_file_open) {
    selected_file.close();
    selected_file_open = 0;
  }
  
  // Open the root directory of the card
  selected_file = SD.open("/");
  selected_file_open = 1;
  
  // Get the first file
  File entry =  selected_file.openNextFile();
  
  // If there was no file, return a no file
  if (! entry) {
    // blank file name is "no files"
    directory_ref_return (NULL, 0);
    Serial.println("No first file");
  }
  else { // there was a file, return the information about the file
    directory_ref_return (entry.name(), entry.size());
    strcpy(filename,entry.name()); // Remember for get_previous call
    entry.close();
    Serial.print("First file:");
    Serial.println(filename);
  }

}

void get_next_directory_entry() {
  Serial.println("Get next directory block");

  // If we never did a get_first, we shouldn't be able to do a get next
  if (! selected_file_open) {
    directory_ref_return (NULL, 0); // return no file
    Serial.println("Next dir: dir not open");
    return;
  }

  // Get the next file in the directory
  File entry =  selected_file.openNextFile();
  
  // If there wasn't one, return no files
  if (! entry) {
    // blank file name is "no files"
    directory_ref_return (NULL, 0);
    Serial.println("Next dir: no more files");
  }
  else { // there was a file, return the information about the file
    directory_ref_return (entry.name(), entry.size());
    strcpy(filename,entry.name()); // Remember for get_previous call
    entry.close();
    Serial.print("Next file:");
    Serial.println(filename);
  }

}

void get_prev_directory_entry() {
  char prev_filename[24];
  int prev_size;
  
  Serial.println("Get prev directory block");

  // If we haven't done a get first, we can't do a get previous
  if (! selected_file_open) {
    directory_ref_return (NULL, 0); // return no files
    Serial.println("Prev dir: dir not open");
    return;
  }
  
  // We can't really go backwards, so we rewind the directory list to the start
  selected_file.rewindDirectory();
  memset(prev_filename, ' ', 24);
  prev_filename[24]=0;
  prev_size=NO_FILE;
  
  // Get the first file in the directory
  File entry =  selected_file.openNextFile();
  
  // While there are files
  while(entry) {
    // If we found the current file
    if (strcmp(entry.name(),filename) == 0) {
      // If we encountered no previous files
      if (prev_size == NO_FILE) {
        directory_ref_return (NULL, 0); // return no file
        entry.close();
        Serial.println("Prev dir: no prev file");
        return;
      }
      else { // return the information about the previous file
        directory_ref_return (prev_filename, prev_size);
        strcpy(filename,prev_filename); // Remember for get_previous call
        entry.close();
        Serial.print("Prev dir:");
        Serial.println(prev_filename);
        return;
      }
    }
    else { // not the file we are looking for
      // We need to remember it's information
      strcpy(prev_filename,entry.name());
      prev_size = entry.size();
    }
    
    entry =  selected_file.openNextFile();
  }
  
  // If we got here, we didn't find the file
  directory_ref_return (NULL, 0);
  Serial.println("Prev dir: No more files");
}

void end_directory_ref() {
  Serial.println("End directory reference");

  // If the directory is still open, close it
  if (selected_file_open) {
    selected_file.close();
    selected_file_open = 0;
  }
  
  // yes, we don't send a responce for this command
}

void directory_ref_return(char *file_name, int file_size)
{
  unsigned short size;
  unsigned char *dotp;
  unsigned char *p;
  unsigned i;

  memset(data,0,31);
  data[29]=0x28; // 40 sectors free
  data[0]=0x11; // return code
  data[1]=0x1C; // length of response

  // If we have a file name to process
  if (file_name)
  {
    // Attribute = 'F'
    data[26] = 'F';

    size = file_size;

    // Put the file size in the buffer
    memcpy (data + 27, &size, 2);
	   
    // Blank out the file name
    memset (data + 2, ' ', 24);

    // Copy the file name to the buffer, upper case it
    for (i = 0; i < min (strlen ((char *)file_name), 24); i++)
    {
      data[2+i] = toupper (file_name[i]);
    }

    // Convert file name from 123.BA to 123   .BA
    dotp = (unsigned char*) memchr (data + 2, '.', 24);
    if (dotp != NULL)
    {
      memmove (data + 6 + 2, dotp, 3);
      for (p = dotp; p < data + 6 + 2; p++)
        *p = ' ';
    }
  }

  // Calculate the checksum
  command_type = 0x11;
  length = 0x1C;
  data[30] = calc_sum ();

  // return the response
  send_data(data,31);
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
    mySerial.write(data[i]);
    Serial.print(data[i],HEX);
    Serial.print(" ");
  }

  Serial.println("");
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

void normal_return(unsigned char type)
{
  command_type = 0x12;
  length = 0x01;
  data[0] = type;
  checksum = calc_sum();
  
  mySerial.write(command_type);
  mySerial.write(length);
  mySerial.write(type);
  mySerial.write(checksum);

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

void dump_data() {
  for(int i = 0; i < bufpos; i++) {
    DEBUG_PRINT1(data[i],HEX);
    DEBUG_PRINT(",");
  }
  DEBUG_PRINTLN("");
}

