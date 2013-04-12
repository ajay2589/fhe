#include "fully_homomorphic.h"

class IsPresent 
{
 private:
  SecuritySettings* sec;
  unsigned int bit_spaces;
  bool found;
  CipherBit** numbers;
  FullyHomomorphic* fh;  
  PrivateKey sk;
  PublicKey pk;
  unsigned int elements;
 public:
  IsPresent(unsigned int bit_spaces);
  void get_line(int line,unsigned int numline); 
  bool if_present();
};
