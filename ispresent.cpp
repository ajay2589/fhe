#include "ispresent.h"
// the number of bits is taken as a command line argument. Default # 3
// LAMBDA can be modified for larger numbers: lambda = 4 works fine for 10 bit integers

#define BIT_LENGTH 12
#define NUM_LINES 100
#define LAMBDA 4

IsPresent::IsPresent(unsigned int bit_spaces) : bit_spaces(bit_spaces) 
{
  sec = new SecuritySettings(LAMBDA);
  fh = new FullyHomomorphic(sec);
  fh->key_gen(sk, pk);
}

void IsPresent::get_line(int line,unsigned int numline) 
{  
  elements = 0;  
  std::vector<CipherBit*> encrypted_numbers_vector;
  CipherBit* encrypted_bit;
  int bin0[BIT_LENGTH] = {0}; 
  int now1 = numline;
  int i = bit_spaces - 1;
  while(now1 != 0)
  {
  	bin0[i] = now1 % 2;
		now1 = now1 / 2;
		i--;
  }
  /*
  printf("Binary form\n");
  i = 0;
  while(i <= 2)
  {
	  printf("%d\t",bin0[i]);
	  i++;
  }
  printf("\n");
  */
  // read every digit in the file  
  int num = 0,temp;
  int numb[NUM_LINES] = {0};  
  FILE *file;
  file = fopen("data", "r");
  if(file == NULL)
    printf("Error: can't open file.\n");
  else
  {
    while(fscanf(file,"%d", &temp) != EOF)
    {
	//	printf("Temp = %d\n",temp);
		numb[num] = temp; 
        num++;
	}
  }
  fclose(file);      
  /*
  printf("Num = %d\n",num);
  for(int jj = 0; jj < num; jj++)
  {
	  printf("%d\t%d\n",jj,numb[jj]);
	  
  }
  */
  // convert them to binary
  int bin1[NUM_LINES][BIT_LENGTH]; 
  int now = 0;
  for(int r = 0;r < 100;r++)  
	  for(int s = 0;s < 10; s++)  
			bin1[r][s] = 0;
  for(int jj = 0; jj < num; jj++)
  {
	  i = bit_spaces - 1;
	  now = numb[jj];
	  while(now != 0)
	  {
		bin1[jj][i] = now % 2;
		now = now / 2;
		i--;
	  }
  }
 
  /*
  for(int jj = 0; jj < num; jj++)
  {
	  i = 0;
	    while(i <= 2)
		{
			printf("%d\t",bin1[jj][i]);
			i++;
		}
		printf("\n");
  }
  */
    
  for (unsigned int i = 0; i < bit_spaces; i++) 
  {
	encrypted_bit = new CipherBit;
	fh->encrypt_bit(*encrypted_bit, pk, bin0[i]);
	fh->print_cipher_bit(*encrypted_bit);
    encrypted_numbers_vector.push_back(encrypted_bit);
  }
  printf("\n");
  elements++;
  for (unsigned int i = 0; i < bit_spaces; i++) 
  {
	encrypted_bit = new CipherBit;
	fh->encrypt_bit(*encrypted_bit, pk, bin1[line][i]);
	fh->print_cipher_bit(*encrypted_bit);
    encrypted_numbers_vector.push_back(encrypted_bit);
  }
  printf("\n");
  elements++;

  numbers = new CipherBit*[encrypted_numbers_vector.size()];  
  for (unsigned int i = 0; i < encrypted_numbers_vector.size(); i++) 
  {
	numbers[i] = encrypted_numbers_vector[i];
  }
}

bool IsPresent::if_present() 
{  
  Gate* input_zero = new Gate(InputLiteral, false, sec);
  Gate* input_one = new Gate(InputLiteral, true, sec);
  Gate* input;  
  unsigned long int w_length = log2(elements);
  unsigned long int p_length = pow(2, w_length);
  Gate* temp_and;
  Gate*** numbers_total = new Gate**[bit_spaces];
  
  for (unsigned int present = 0; present < bit_spaces; present++) 
  {
	Gate*** p = new Gate**[elements+1];
	for (unsigned long int i = 0; i < elements+1; i++) 
	{ //row
	  p[i] = new Gate*[p_length+1];
	  p[i][0] = input_one;
	  if (i > 0) 
	  {
		input = new Gate(Input, (i-1)*bit_spaces + present, sec);
	  }
	  for (unsigned int j = 1; j < p_length+1; j++) 
	  { //col
		if (i == 0) 
		{
		  p[0][j] = input_zero;
		} 
		else 
		{
		  temp_and = new Gate(And, p[i-1][j-1], input, sec);
		  p[i][j] = new Gate(Xor, temp_and, p[i-1][j], sec);
		}
	  }
	}
	numbers_total[present] = new Gate*[w_length+1];
	int cur_index = 0;
	for (unsigned long int i = p_length; i > 0; i >>= 1) 
	{
	  numbers_total[present][cur_index++] = p[elements][i];
	}
  }
  Gate* output;
  std::vector<Gate*> output_gates;
  for (unsigned int present = 0; present < bit_spaces; present++) 
  {
	for (unsigned long int i = 0; i < w_length+1; i++) 
	{
	  output = new Gate(Output, numbers_total[present][i], sec);
	  output_gates.push_back(output);
	}
  }

  CipherBit** encrypted_results = fh->evaluate(output_gates, numbers, pk);
  bool* decrypted_results = fh->decrypt_bit_vector(sk, encrypted_results, bit_spaces*(w_length+1));
  int ress[NUM_LINES][BIT_LENGTH];
  textcolor(BRIGHT, RED);    
  for (unsigned int present = 0; present < bit_spaces; present++) 
  {	
	for (unsigned int i = 0; i < w_length+1; i++) 
	{
	  ress[present][w_length - i] = decrypted_results[present*(w_length+1) + i];
	}
  }
  unsigned int ssum[NUM_LINES]= {0};	
  for (unsigned int present = 0; present < bit_spaces; present++) 
  {
	unsigned int pow2 = 1,kk = 0;	
	for(kk = 0; kk <= w_length; kk++)
	{
	  ssum[present] = ssum[present] + (ress[present][kk] * pow2);
	  pow2 = pow2 * 2;
	}
  }
  /*
  for(unsigned int present = 0; present < bit_spaces; present++)
  {
	printf("%d\t",ssum[present]);
  }
  printf("\n");
  */
  bool found = true;
  for(unsigned int present = 0; present < bit_spaces; present++)
  {
	if((ssum[present] % 2) != 0)	
		found = false;
  }
  if(found)
  {
	printf("The element is present\n");		
	resettextcolor();
	exit(-9);
  }
  resettextcolor();
  return found;
}

int main(int argc, char** argv) 
{ 
  unsigned int bit_spaces = 0; 
  if (argc != 2) 
	bit_spaces = 3; 	  
  else
	bit_spaces = atoi(argv[1]);
  bool found = true;  
  IsPresent demo(bit_spaces);
  unsigned int numline;
  printf("Please enter the number to be searched: ");
  scanf("%u", &numline);  
  FILE *file;
  int temp, num = 0;
  file = fopen("data", "r");
  if(file == NULL)
    printf("Error: can't open file.\n");
  else
  {
    while(fscanf(file,"%d", &temp) != EOF)
    {		
        num++;
	}
  }  
  for(int ii = 0; ii < num; ii ++)
  {
	demo.get_line(ii,numline);  
	found = demo.if_present();
  } 
  
  textcolor(BRIGHT, RED);
  if(!found)
	printf("Element not present \n");
  resettextcolor();	
}
