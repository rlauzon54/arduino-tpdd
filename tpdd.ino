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
int selected_file_mode;
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
  // (53 on the Mega) must be left as an output or the SD library 
  // functions will not work. 
  // Note that we connect to this pin through the SPI header.  The SD
  // shield doesn't stretch to pin 53 on the Mega.
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
  selected_file_mode = 0;

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
      case initial_state: // We started getting data
        preamble[0] = mySerial.read();
        DEBUG_PRINTLN("start preamble");
        state = start_preamble;
        digitalWrite(HDD_IND,HIGH);  // turn the drive indicator on
        break;
        
      case start_preamble: // second char read
        preamble[1] = mySerial.read();
        DEBUG_PRINT("got preamble:");
        DEBUG_PRINT(preamble[0]);
        DEBUG_PRINTLN(preamble[1]);
        state = got_preamble; // we have the pre-amble
        break;
      
      case got_preamble:
      // If the pre-amble is junk, reset communications
        if (preamble[0] != 'Z' or preamble[1] != 'Z') {
          state=initial_state;  // Ignore the input
          digitalWrite(HDD_IND,LOW);  // turn the drive indicator off        
          DEBUG_PRINT("BAD preamble:");
          DEBUG_PRINT(preamble[0]);
          DEBUG_PRINTLN(preamble[1]);
        }
        else { // we got good stuff - read the command type
          command_type = mySerial.read();
          state=got_command_type;
          DEBUG_PRINT("got command type:");
          DEBUG_PRINTLN1(command_type,HEX);
        }
        break;
        
      case got_command_type: // we got the command type
        length = mySerial.read(); // read the data length
        state = getting_data;
        DEBUG_PRINT("got length:");
        DEBUG_PRINTLN1(length,HEX);
        bufpos=0;
        break;
      
      case getting_data: // we have a length
        // Did we get all the data?
        if (bufpos >= length) {
          // yes, now read the checksum
          checksum = mySerial.read();
          state = processing_command; // indicate that we got all the information we need to process the command
          DEBUG_PRINT("got data - Checksum:");
          DEBUG_PRINTLN1(checksum,HEX);
        }
        else { // Still reading
          // Get the next char and put it away
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
  
  // We have all the data for processing the command
    if (state == processing_command) {
        DEBUG_PRINT("Processing command type = ");
        DEBUG_PRINT1(command_type,HEX);
        DEBUG_PRINT(" length = ");
        DEBUG_PRINT1(length,HEX);
        DEBUG_PRINT(" csum = ");
        DEBUG_PRINTLN1(checksum,HEX);
    
        // validate checksum
        if (calc_sum() != checksum) {
          DEBUG_PRINT("Bad checksum: ");
          DEBUG_PRINTLN1(calc_sum(),HEX);
          dump_data();
          normal_return(0x36);
          state=initial_state;
          digitalWrite(HDD_IND,LOW);  // turn the drive indicator off
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
            delete_file();
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
            Serial.print("Unknown command type:");
            Serial.println(command_type,HEX);
            break;
    } // Command type switch
    
    state=initial_state;
    digitalWrite(HDD_IND,LOW);

  }
}

void process_directory_command() {
  DEBUG_PRINT("Processing directory ref: ");

  int search_form = data[length-1];
  DEBUG_PRINTLN1(search_form,HEX);
  
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
  
  DEBUG_PRINT("Picked file:");
  DEBUG_PRINTLN(filename);
  
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
    DEBUG_PRINTLN("No first file");
  }
  else { // there was a file, return the information about the file
    directory_ref_return (entry.name(), entry.size());
    strcpy(filename,entry.name()); // Remember for get_previous call
    entry.close();
    DEBUG_PRINT("First file:");
    DEBUG_PRINTLN(filename);
  }

}

void get_next_directory_entry() {
  DEBUG_PRINTLN("Get next directory block");

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
    DEBUG_PRINTLN("Next dir: no more files");
  }
  else { // there was a file, return the information about the file
    directory_ref_return (entry.name(), entry.size());
    strcpy(filename,entry.name()); // Remember for get_previous call
    entry.close();
    DEBUG_PRINT("Next file:");
    DEBUG_PRINTLN(filename);
  }
}

void get_prev_directory_entry() {
  char prev_filename[24];
  int prev_size;
  
  DEBUG_PRINTLN("Get prev directory block");

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
  
  // While there are files in the directory
  while(entry) {
    // If we found the current file we looked at
    if (strcmp(entry.name(),filename) == 0) {
      // If we encountered no previous files
      if (prev_size == NO_FILE) {
        directory_ref_return (NULL, 0); // return no file
        entry.close();
        DEBUG_PRINTLN("Prev dir: no prev file");
        return;
      }
      else { // return the information about the previous file
        directory_ref_return (prev_filename, prev_size);
        strcpy(filename,prev_filename); // Remember for get_previous call
        entry.close();
        DEBUG_PRINT("Prev dir:");
        DEBUG_PRINTLN(prev_filename);
        return;
      }
    }
    else { // not the file we are looking for
      // We need to remember it's information
      strcpy(prev_filename,entry.name());
      prev_size = entry.size();
    }
    
    // next directory entry
    entry =  selected_file.openNextFile();
  }
  
  // If we got here, we didn't find the file
  directory_ref_return (NULL, 0);
  DEBUG_PRINTLN("Prev dir: No more files");
}

void end_directory_ref() {
  DEBUG_PRINTLN("End directory reference");

  // If the directory is still open, close it
  if (selected_file_open) {
    selected_file.close();
    selected_file_open = 0;
  }
  
  // yes, we don't send a response for this command
}

void directory_ref_return(char *file_name, int file_size)
{
  unsigned short size;
  unsigned char *dotp;
  unsigned char *p;
  unsigned i;

  memset(data,0,31);

  // If we have a file name to process
  if (file_name)
  {
    // Blank out the file name
    memset (data, ' ', 24);

    // Copy the file name to the buffer, upper case it
    for (i = 0; i < min (strlen ((char *)file_name), 24); i++)
    {
      data[i] = toupper (file_name[i]);
    }

    // Convert file name from 123.BA to 123   .BA
    dotp = (unsigned char*) memchr (data, '.', 24);
    if (dotp != NULL)
    {
      memmove (data + 6, dotp, 3);
      for (p = dotp; p < data + 6; p++)
        *p = ' ';
    }

    // Attribute = 'F'
    data[24] = 'F';

    size = file_size;

    // Put the file size in the buffer
    memcpy (data + 25, &size, 2);	   
  }

  data[27]=0x28; // 40 sectors free

  // Send the data back
  command_type = 0x11;
  length = 0x1C;
  send_data(command_type,data,28,calc_sum ());
}

void open_file(int omode)
{
  DEBUG_PRINT("Processing open file:");
  DEBUG_PRINTLN(filename);
  
  // If we have a file open, close it
  if (selected_file_open) {
    selected_file.close();
    selected_file_open = 0;
  }
  
  switch(omode) {
    case 0x01:  /* New file for my_write */
      DEBUG_PRINTLN("New file for write");
      
      // if the file exists, remove it
      if (SD.exists(filename)) {
        DEBUG_PRINTLN("Removing old file");
        SD.remove(filename);
      }
      // Fall through
      
    case 0x02:  /* existing file for append */
      DEBUG_PRINTLN("Opening file for append");
      selected_file = SD.open(filename,FILE_WRITE);
      selected_file_open=1;
      break;
      
    case 0x03:  /* Existing file for read */
      DEBUG_PRINTLN("Existing for read");
      selected_file = SD.open(filename,FILE_READ);
      selected_file_open=1;
      break;
    }
    
    // If the file didn't open
    if (!selected_file) {
      selected_file_open =0;
      normal_return (0x37); // Error opening file
    }
    else {
      // If the file is too large
      if (selected_file.size() > 65535) {
        selected_file_open =0;
        normal_return (0x6E);// file too long error
      }
      else { // everything's good
        selected_file_open =1;
        normal_return (0x00);
        selected_file_mode = omode;
      }    
    }
}

void close_file() {
  DEBUG_PRINTLN("Processing close file");

  if (selected_file_open) {
    selected_file.close();
    selected_file_open = 0;
  }
  
  normal_return(0x00);
}

void read_file() {
  DEBUG_PRINTLN("Processing read file");
  
  // return type 10
  command_type=0x10;

  // If the file is not open
  if(! selected_file_open)
  {
    normal_return(0x30); // no file name error
    return;
  }

  // If we didn't open the file for read
  if(selected_file_mode!=3)
  {
    normal_return(0x37);  // open format mismatch
    return;
  }

  // get the next 128 bytes from the file
  length = selected_file.read(data, 128);

  // Calculate the check sum of the message
  send_data(command_type, data, length, calc_sum());
}

void write_file() {
  DEBUG_PRINTLN("Processing write file");

  // if the file is not open
  if (!selected_file_open) {
    normal_return(0x30); // no file name
    return;
  }
			
  // If the open mode is not write
  if(selected_file_mode!=1 && selected_file_mode !=2) {
    normal_return(0x37); // open format mismatch
    return;
  }
			
  // Write the data out to the file
  if(selected_file.write(data,length)!=length) {
    normal_return(0x4a); // sector number error
  }
  else
  {
    normal_return(0x00); // everything's OK
  }
			
}

void delete_file() {
  DEBUG_PRINTLN("Processing delete file");
  
  // If the file is open, remove it
  if (selected_file_open) selected_file.close();
  
  // Remove the file
  SD.remove(filename);
  
  normal_return(0x00);
}

void rename_file() {
  DEBUG_PRINTLN("Processing file rename command");
  char newfile_name[24];
  int in;
  
  // Get the new file name from the input
  memcpy(newfile_name,data,24);
  newfile_name[24]=0;
  
  // Remove trailing spaces
  for(int i=23; i > 0 && newfile_name[i] == ' '; i--) {
    newfile_name[i] = 0;
  }

  char *dot;
  char *p;
  /* Remove spaces between base and dot */
  dot = strchr((char *)newfile_name,'.');
  if(dot != NULL) {
    for(p=dot-1;*p==' ';p--);
      memmove(p+1,dot,strlen((char *)dot)+1);
  }
  
  // You can't rename a file to something that's already there
  if (SD.exists(newfile_name)) {
    normal_return(0x4A); // sector number error
    return;
  }

  // There's no SD method for renaming a file.
  // So we open the new file, copy the data from the old file, then delete the old file
  
  File oldfile = SD.open(filename,FILE_READ);
  File newfile = SD.open(newfile_name,FILE_WRITE);
  
  in = oldfile.read(data,200);
  while (in == 200) {
    newfile.write(data,200);
    in = oldfile.read(data,200);
  }
  if (in > 0) {
    newfile.write(data,in);
  }

  newfile.close();
  oldfile.close();
  
  SD.remove(filename);
}

void respond_place_path()
{
  DEBUG_PRINTLN("Processing drive condition command");
  data[0] = 0x00;
  data[1] = 0x52;
  data[2] = 0x4f;
  data[3] = 0x4f;
  data[4] = 0x54;
  data[5] = 0x20;
  data[6] = 0x20;
  data[7] = 0x2e;
  data[8] = 0x3c;
  data[9] = 0x3e;
  data[10] = 0x20;
  send_data(0x12,data,11, 0x96);
}

void respond_mystery()
{
  DEBUG_PRINTLN("Processing mystery command 1");
  data[0] = 0x00;
  send_data(0x38,data,1,0xC6);
}

void respond_mystery2()
{
  DEBUG_PRINTLN("Processing mystery command 2");
  data[0] = 0x41;
  data[1] = 0x10;
  data[2] = 0x01;
  data[3] = 0x00;
  data[4] = 0x50;
  data[5] = 0x05;
  data[6] = 0x00;
  data[7] = 0x02;
  data[8] = 0x00;
  data[9] = 0x28;
  data[10] = 0x00;
  data[11] = 0xE1;
  data[12] = 0x00;
  data[13] = 0x00;
  data[14] = 0x00;
  send_data(0x14, data, 15, 0x2A);
}

void send_data(char return_type, unsigned char data[], int length, int checksum) {
  mySerial.write(return_type);
  DEBUG_PRINT1(return_type,HEX);
  DEBUG_PRINT(" ");

  mySerial.write(length);
  DEBUG_PRINT1(length,HEX);
  DEBUG_PRINT(" ");
  
  for(int i=0; i < length; i++) {
    mySerial.write(data[i]);
    if (data[i] > 31 && data[i] < 127) {
      DEBUG_PRINT((char)data[i]);
    }
    else {
      DEBUG_PRINT("0x");
      DEBUG_PRINT1(data[i],HEX);
    }
    DEBUG_PRINT(" ");
  }

  mySerial.write(checksum);
  DEBUG_PRINT1(checksum,HEX);
  DEBUG_PRINTLN("");
}

void normal_return(unsigned char type)
{
  command_type = 0x12;
  length = 0x01;
  data[0] = type;
  
  send_data(command_type, data, length, calc_sum());
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

