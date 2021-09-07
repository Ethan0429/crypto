#include<iostream>
#include<string>
#include<vector>
#include<unistd.h>
#include<algorithm>

#define ENCRYPT 'e'
#define DECRYPT 'd'
#define QUIT    'q'

unsigned char getChar(FILE* f) {
  auto off = ftell(f);
  unsigned char c = fgetc(f);
  fseek(f, off, SEEK_SET);
  return c;
}

unsigned char rand_char() {
  unsigned char c = (rand() % 127) + 31;
  return c;
}

std::string rand_string( size_t length )
{
    auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
}

long int getFileSize(FILE* &f) {
  fseek(f, 0, SEEK_END);
  auto size = ftell(f);
  rewind(f);
  return size;
}

bool file_exists(FILE* f) {
  if (f)
    return true;
  else
    return false;
}

bool encrypt(char file[]) {
  srand(time(0));
  // vector of encrypted data
  std::vector<unsigned char> bytes;
  // open file
  FILE* f = fopen(file, "r+b");
  if (!file_exists(f)) {
    printf("\n(%s) ",file);    
    perror("Error: ");
    return false;
  }
  // get file size
  auto size = getFileSize(f);
  // encrypt file byte by byte
  for (int i=0;i<size;i++) {
    unsigned char c = getChar(f); // original character
    unsigned char x = rand_char(); // key character
    unsigned char put = c - x; // new character
    bytes.push_back(x); // push key character to byte array
    putc(put, f); // replace original char with new char
  }

  // generate random file name for decryption key
  std::string key_file = (rand_string(10) + ".bin");
  // append key file name to original file
  for (auto c : key_file) {
    putc(c, f);
  }
  // close encrypted file
  fclose(f);
  
  // open key file and write the byte array to it
  f = fopen(key_file.c_str(), "w+b");
  if (!file_exists(f)) {
    printf("\n(%s) ",file);    
    perror("Error: ");
    return false;
  }
  for (auto b : bytes) {
    putc(b, f);
  }
  // close key file
  fclose(f);
  printf("%s encrypted!\n\n",file);

  // exit success
  return true;
}

bool decrypt(char file[]) {

  // open encrypted file
  FILE* f = fopen(file, "r+b");
  if (!file_exists(f)) {
    printf("\n(%s) ",file);    
    perror("Error: ");
    return false;
  }
  // create bytes array
  std::vector<unsigned char> bytes;
  // get size of file
  auto size = getFileSize(f);
  // seek to end of file to read key file name
  fseek(f, -14, SEEK_END);
  // create key buffer
  char key[14];
  // read key file name into key buffer
  fread(key, 14, 1, f);
  // close encrypted file
  fclose(f);

  // open key file for reading
  f = fopen(key, "r+b");
  if (!file_exists(f)) {
    printf("\n(%s) ",key);    
    perror("Error: ");    
    return false;
  }
  size = getFileSize(f);
  // read bytes of key file & push into bytes array
  for (int i=0;i<size;i++) {
    bytes.push_back(fgetc(f));
  }
  // close key file
  fclose(f);

  // open original file for writing
  f = fopen(file, "r+b");
  if (!file_exists(f)) {
    printf("\n(%s) ",file);    
    perror("Error: ");    
    return false;
  }
  size = getFileSize(f);
  // add key bytes to encrypted file bytes (decrypt)
  for (int i=0;i<size-14;i++) {
    unsigned char c = getChar(f);
    putc(c + bytes[i], f);
  }
  // truncate appended key file name from decrypted txt
  truncate(file, size-14);
  // close decrypted file
  remove(key);
  fclose(f);
  printf("%s decrypted!\n\n",file);
  
  // exit success
  return true;
}

int main() {
  char file[255];
  char option;

  printf("~ File Encryption/Decryption ~\n");
  printf("Options:\nq - quit\ne - encrypt\nd - decrypt\n\n");

  while(1) {
    option = getc(stdin);
    switch(option) {
      case ENCRYPT:
        printf("\nEnter file to encrypt: ");
        scanf("%s", file);
        if (!encrypt(file))
          return 0;
        break;
      case DECRYPT:
        printf("\nEnter file to decrypt: ");
        scanf("%s", file);
        if (!decrypt(file))
          return 0;
        break;
      case QUIT:
        std::cout << "\nGoodbye!\n";
        return 0;
      case '\n':
        break;
      default:
        std::cout << "Input not recognized\nOptions:\nq - quit\ne - encrypt\nd - decrypt\n\n";
        break;
    }
  }
}