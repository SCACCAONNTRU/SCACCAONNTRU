/*
   PQCgenKAT_kem.c
   Created by Bassham, Lawrence E (Fed) on 8/29/17.
   Copyright © 2017 Bassham, Lawrence E (Fed). All rights reserved.
   + mods from djb: see KATNOTES
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rng.h"
#include "crypto_kem.h"
#include "attack_parameters.h"
#include "math.h"
#include "int8.h"
#include "int16.h"
#include "int32.h"
#include "uint16.h"
#include "uint32.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wparentheses-equality"
#pragma GCC diagnostic ignored "-Wimplicit-int"
#pragma GCC diagnostic ignored "-Wunused-label"

double mean_now = 0;
double std_dev = 2;

static int shift_lfsr(unsigned int *lfsr, unsigned int polynomial_mask)
{
    int feedback;

    feedback = *lfsr & 1;
    *lfsr >>= 1;
    if(feedback == 1)
        *lfsr ^= polynomial_mask;
    return *lfsr;
}

static int get_random(void)
{
    int temp;
    unsigned int POLY_MASK_HERE_1 = 0x121212AB;
    unsigned int POLY_MASK_HERE_2 = 0xABBBEECD;
    static unsigned int lfsr_1 = 0x55AAEEFF;
    static unsigned int lfsr_2 = 0xFFAA8844;
    shift_lfsr(&lfsr_1, POLY_MASK_HERE_1);
    shift_lfsr(&lfsr_2, POLY_MASK_HERE_2);
    temp = (shift_lfsr(&lfsr_1, POLY_MASK_HERE_1) ^ shift_lfsr(&lfsr_2, POLY_MASK_HERE_2)) & 0XFF;
    return (temp);
}

/* x must not be close to top int32 */
static Fq Fq_freeze(int32 x)
{
  return int32_mod_uint14(x+q12,q)-q12;
}

#define KAT_SUCCESS          0
#define KAT_FILE_OPEN_ERROR -1
#define KAT_CRYPTO_FAILURE  -4
#define NTESTS 1

//----- Defines -------------------------------------------------------------
#define PI         3.14159265   // The value of pi

//----- Function prototypes -------------------------------------------------
double norm(double mean, double std_dev);  // Returns a normal rv
double rand_val(int seed);                 // Jain's RNG

//===========================================================================
//=  Function to generate normally distributed random variable using the    =
//=  Box-Muller method                                                      =
//=    - Input: mean and standard deviation                                 =
//=    - Output: Returns with normally distributed random variable          =
//===========================================================================
double norm(double mean, double std_dev)
{
  double   u, r, theta;           // Variables for Box-Muller method
  double   x;                     // Normal(0, 1) rv
  double   norm_rv;               // The adjusted normal rv

  // Generate u
  u = 0.0;
  while (u == 0.0)
    u = ((double)(get_random()*256 + get_random()))/65536;

  // Compute r
  r = sqrt(-2.0 * log(u));

  // Generate theta
  theta = 0.0;
  while (theta == 0.0)
    theta = 2.0 * PI * ((double)(get_random()*256 + get_random()))/65536;

  // Generate x value
  x = r * cos(theta);
  // printf("u: %f, x: %f, r: %f\n, theta: %d\n", u, x, r, theta);

  // Adjust x value for specified mean and variance
  norm_rv = (x * std_dev) + mean;

  // Return the normally distributed RV value
  return(norm_rv);
}

//=========================================================================
//= Multiplicative LCG for generating uniform(0.0, 1.0) random numbers    =
//=   - x_n = 7^5*x_(n-1)mod(2^31 - 1)                                    =
//=   - With x seeded to 1 the 10000th x value should be 1043618065       =
//=   - From R. Jain, "The Art of Computer Systems Performance Analysis," =
//=     John Wiley & Sons, 1991. (Page 443, Figure 26.2)                  =
//=========================================================================
double rand_val(int seed)
{
  const long  a =      16807;  // Multiplier
  const long  m = 2147483647;  // Modulus
  const long  q_now =     127773;  // m div a
  const long  r =       2836;  // m mod a
  static long x;               // Random int value
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // Set the seed if argument is non-zero and then return zero
  if (seed > 0)
  {
    x = seed;
    return(0.0);
  }

  // RNG using integer arithmetic
  x_div_q = x / q_now;
  x_mod_q = x % q_now;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + m;

  // Return a random value between 0.0 and 1.0
  return((double) x / m);
}

// void	fprintBstr(FILE *fp, char *S, unsigned char *A, unsigned long long L);

static uint8_t hw_calc(int8_t byte)
{
  uint8_t bit;
  uint8_t weight = 0;
  for(int i = 0; i < 8; i++)
  {
    bit = (byte >> i)&0x1;
    weight = weight+bit;
  }
  return weight;
}

extern int no_true_collisions;
int intended_function;
int sec_index;

extern int check_for_value;
extern int collision_index;
extern int collision_value;
extern int error_now;
extern int hw_value;
extern int no_leakage_trials;
extern int m;
extern int n;

extern int c_value_for_attack_1_1;
extern int c_value_for_attack_1_2;
extern int c_value_for_attack_1_3;

extern int c_value_for_attack_2_1;
extern int c_value_for_attack_2_2;
extern int c_value_for_attack_2_3;

extern int c1_value_1, c1_value_2, c1_value_3;
extern int c2_value_1, c2_value_2, c2_value_3;

unsigned char temp_temp_char;

no_leakage_trials = 100;

int collision_array_value[count_threshold];
int collision_array_index[count_threshold];

unsigned char entropy_input[48];
unsigned char seed[NTESTS][48];

int main()
{
    FILE                *fp_req, *fp_rsp;
    int                 ret_val;
    int i;
    unsigned char *ct = 0;
    unsigned char *ss = 0;
    unsigned char *ss1 = 0;
    unsigned char *pk = 0;
    unsigned char *sk = 0;

    for (i=0; i<48; i++)
        entropy_input[i] = get_random()&0xFF;
    randombytes_init(entropy_input, NULL, 256);

    for (i=0; i<NTESTS; i++)
        randombytes(seed[i], 48);

    int found_c = 0;
    int c_value_current;
    int final_coeff_to_find;


    FILE * f2;
    FILE * f3;

    char ct_file_now[30];
    char ct_file_now_basic[30];
    char keypair_file_now[30];
    char oracle_responses_now_file_name[30];
    char valid_ct_file_now[30];
    char ct_file_basic_failed[50];

    int weight_for_secret;

    #if (DO_PRINT == 1)

    // We can store the data of a single iteration in files...
    // Please note that these files will be overwritten for every iteration...

    // We store the oracle responses in oracle_resp.bin...

    sprintf(oracle_responses_now_file_name,"oracle_resp_file.bin");

    // Here, we store the attack ciphertexts...

    sprintf(ct_file_now,"ct_file.bin");

    // Here, we store the base ciphertext...

    sprintf(ct_file_now_basic,"ct_file_basic.bin");

    // Here, we store the valid ciphertext...

    sprintf(valid_ct_file_now,"valid_ct_file.bin");

    // Here, we store the public and private key pair...

    sprintf(keypair_file_now,"keypair_file.bin");

    // Here, we store the failed ciphertexts which do not correspond to any collision...

    sprintf(ct_file_basic_failed,"ct_file_basic_failed.bin");

    #endif

    // This is where we set the value of the number of non-zero coefficients of the polynomial d1 (m) and d2 (n) in the base ciphertext...
    // The value for M_VALUE and N_VALUE is set in the crypto_kem.h file.

    m = M_VALUE;
    n = N_VALUE;

    int max_distance = 1000000;
    int max_distance2 = 0;

    max_distance2 = 0;

    while(found_c == 0)
    {
      for(int hg = 0; hg < q; hg++)
      {

        for(int hg1 = 0; hg1 < q; hg1++)
        {

              c_value_current = hg;
              int value1 = hg * (3*2*m) + hg1 * (2*n);

              int touch_np = 0;

              max_distance = 1000000;

              for(int poss = 0; poss <= 2*m; poss++)
              {
                for(int poss1 = 0; poss1 <= 2*n; poss1++)
                {
                      if(!(poss == 2*m && poss1 == 2*n))
                      {

                        int value2 = hg * (3*poss) + hg1*poss1;

                        if(max_distance > (abs(q12 - value2)))
                          max_distance = (abs(q12 - value2));

                        if((value1 < q12) || (value2 > q12) || (hg%3 != 0) || (hg1%3 != 0) || (abs(q12 - value1) < C_VALUE_THRESHOLD_1) || (abs(q12 - value2) < C_VALUE_THRESHOLD_2))
                        {
                          touch_np = 1;
                        }

                      }
                }
              }

              if(touch_np == 0)
              {
                found_c = 1;

                if(max_distance > max_distance2)
                {
                  c_value_1 = hg;
                  c_value_2 = hg1;
                  max_distance2 = max_distance;
                  printf("hg = %d, hg1 = %d, Diff1: %d, Diff2: %d\n", hg, hg1, abs(q12 - value1), max_distance2);
                }

              }

        }
      }
    }

    printf("Found c light\n");

    // This is used to calculate k1 and k2 for the base ciphertext cbase, as described in the paper...

    found_c = 0;

    max_distance2 = 0;
    while(found_c == 0)
    {
      for(int hg = 0; hg < q; hg++)
      {

        for(int hg1 = 0; hg1 < q; hg1++)
        {

              int value1 = hg * (3*2*m) + hg1 * (2*n);

              int touch_np = 0;

              max_distance = 1000000;

              for(int poss = 0; poss <= 2*m; poss++)
              {
                for(int poss1 = 0; poss1 <= 2*n; poss1++)
                {
                      if(!(poss == 2*m && poss1 == 2*n))
                      {

                        int value2 = hg * (3*poss) + hg1*poss1;

                        if(max_distance > (abs(q12 - value2)))
                          max_distance = (abs(q12 - value2));

                        if((value1 > q12) || !((abs(value1 - q12) > C_VALUE_THRESHOLD_1_TRIMMER_LESS) && (abs(value1 - q12) < C_VALUE_THRESHOLD_1_TRIMMER_GREAT))
                          || (value2 > q12) || (hg%3 != 0) || (hg1%3 != 0) || (abs(q12 - value2) < C_VALUE_THRESHOLD_2_TRIMMER))
                        {
                          touch_np = 1;
                        }

                      }
                }
              }

              if(touch_np == 0)
              {
                found_c = 1;

                if(max_distance > max_distance2)
                {
                  c_value_1_trimming = hg;
                  c_value_2_trimming = hg1;
                  max_distance2 = max_distance;
                  printf("hg = %d, hg1 = %d, Diff1: %d, Diff2: %d\n", hg, hg1, abs(q12 - value1), max_distance2);
                }

              }

        }
      }
    }


    printf("Found c trim...\n");

    // Here, we are trying to compute l1, l2 and l3 for the attack ciphertexts as shown in the paper...
    // We compute l1, l2, l3 is used to distinguish 1...

    int list_of_c1_values[30][10];
    int list_of_c2_values[30][10];


    int no_c1_values = 0;
    int no_c2_values = 0;

    int found_c_for_attack_1 = 0;
    int found_c_for_attack_2 = 0;

    int sample_1, sample_2, sample_3;


    max_distance = 1000000;
    max_distance2 = 0;

    int limit_value = 1000;

    while(found_c_for_attack_1 == 0)
    {
      for(int hg = 0; hg < limit_value; hg = hg+3)
      {

        for(int hg1 = 0; hg1 < limit_value; hg1 = hg1+3)
        {

          for(int hg2 = 0; hg2 < limit_value; hg2 = hg2+3)
          {

            sample_1 = hg;
            sample_2 = hg1;
            sample_3 = hg2;

            int value1 = sample_1 * (3*2*m) + sample_3 * (2*n) + sample_2 * (3*1);
            int value2;

            int touch_np = 0;

            max_distance = 1000000;


            for(int poss = 0; poss <= 2*m; poss++)
            {
              for(int poss1 = 0; poss1 <= 2*n; poss1++)
              {
                for(int poss2 = 0; poss2 <= 2; poss2++)
                {

                  if(!((poss == 2*m) && (poss1 == 2*n) && ((poss2 == 2) || (poss2 == 1))))
                  {

                    value2 = sample_1 * (3*poss) + sample_3 * poss1 + sample_2 * (3*poss2);

                    if(max_distance > (abs(q12 - value2)))
                      max_distance = (abs(q12 - value2));

                    if((value1 < q12) || (value2 > q12)
                      || (abs(value1 - q12) < GAP_THRESHOLD_1_1) || (abs(value2 - q12) < GAP_THRESHOLD_1_2))
                    {
                      touch_np = 1;
                    }
                  }

                }
              }
            }

            if(touch_np == 0)
            {
              found_c_for_attack_1 = 1;

              if(max_distance > max_distance2)
              {
                c1_value_1 = hg;
                c1_value_2 = hg1;
                c1_value_3 = hg2;

                max_distance2 = max_distance;
                printf("hg = %d, hg1 = %d, hg2 = %d, Diff1: %d, Diff2: %d\n", hg, hg1, hg2, abs(q12 - value1), max_distance2);
              }

            }
          }
        }
      }
    }

    printf("Values...\n");
    for(int bv = 0; bv < no_c1_values; bv++)
    {
      printf("%d, %d, %d\n", list_of_c1_values[bv][0],list_of_c1_values[bv][1],list_of_c1_values[bv][2]);
    }

    printf("Found c_1\n");

    // Here, we are trying to compute l1, l2 and l3 for the attack ciphertexts as shown in the paper...
    // We compute l1, l2, l3 is used to distinguish 2...

    max_distance2 = 0;

    limit_value = 1000;

    while(found_c_for_attack_2 == 0)
    {

      for(int hg = 0; hg < limit_value; hg = hg+3)
      {

        for(int hg1 = 0; hg1 < limit_value; hg1 = hg1+3)
        {

          for(int hg2 = 0; hg2 < limit_value; hg2 = hg2+3)
          {

            sample_1 = hg;
            sample_2 = hg1;
            sample_3 = hg2;

            int value1 = sample_1 * (3*2*m) + sample_3 * (2*n) + sample_2 * (3*2);

            int value2;

            int touch_np = 0;
            max_distance = 1000000;

            for(int poss = 0; poss <= 2*m; poss++)
            {
              for(int poss1 = 0; poss1 <= (2*n); poss1++)
              {
                for(int poss2 = 0; poss2 <= 2; poss2++)
                {

                  if(!((poss == 2*m) && (poss1 == (2*n)) && (poss2 == 2)))
                  {
                    value2 = sample_1 * (3*poss) + sample_3 * poss1 + sample_2 * (3*poss2);

                    if(max_distance > (abs(q12 - value2)))
                      max_distance = (abs(q12 - value2));

                    if((value1 < q12) || (value2 > q12)
                      || (abs(value1 - q12) < GAP_THRESHOLD_2_1) || (abs(value2 - q12) < GAP_THRESHOLD_2_2))
                    {
                      touch_np = 1;
                    }

                  }
                }
              }
            }

            if(touch_np == 0)
            {
              found_c_for_attack_2 = 1;


              if(max_distance > max_distance2)
              {

                c2_value_1 = hg;
                c2_value_2 = hg1;
                c2_value_3 = hg2;

                max_distance2 = max_distance;

                printf("hg = %d, hg1 = %d, hg2 = %d, Diff1: %d, Diff2: %d\n", hg, hg1, hg2, abs(q12 - value1), max_distance2);
              }

            }
          }
        }
      }
    }

    printf("Found c_2\n");

    // So, we basically get two values for the (l1, l2, l3)... Let us denote them as (l11, l12, l13) and (l21, l22, l23)...
    // The attack ciphertexts are c = l1. d1 + l2. d2 . h + l3. x^u. (x-1)...

    //       (l11,l12,l13)     (l21,l22,l23)      (l11,l12,-l13)      (l21,l22,-l23)
    // 2         O                  O                   X                  X
    // 1         O                  O                   X                  O
    // 0         O                  O                   O                  O
    // -1        X                  O                   O                  O
    // -2        X                  X                   O                  O



    c_value_for_attack_1_1 = c1_value_1;
    c_value_for_attack_1_2 = c1_value_2;
    c_value_for_attack_1_3 = c1_value_3;

    c_value_for_attack_2_1 = c2_value_1;
    c_value_for_attack_2_2 = c2_value_2;
    c_value_for_attack_2_3 = c2_value_3;


    double profile_average_count = 0;
    double trace_average_count = 0;

    if (!ct) ct = malloc(crypto_kem_CIPHERTEXTBYTES);
    if (!ct) abort();
    if (!ss) ss = malloc(crypto_kem_BYTES);
    if (!ss) abort();
    if (!ss1) ss1 = malloc(crypto_kem_BYTES);
    if (!ss1) abort();
    if (!pk) pk = malloc(crypto_kem_PUBLICKEYBYTES);
    if (!pk) abort();
    if (!sk) sk = malloc(crypto_kem_SECRETKEYBYTES);
    if (!sk) abort();

    // Iterate over the number of tests you want to run... The NO_TESTS variable is defined in params.h header file...

    int multiple_collision_count = 0;
    int rounding_error_return = 0;
    int count_non_zero_coeffs = 0;
    int total_profile_trials_overall = 0;

    int no_single_collisions = 0;
    int no_multiple_collisions = 0;
    int no_false_negative_collisions = 0;
    int no_false_positive_collisions = 0;

    for (int pq=0; pq<NO_TESTS; pq++)
    {
        printf("Trial: %d\n",pq);

        // Please note that the data is overwritten for every attack trial... So, only the data for the last trial will be stored...

        #if (DO_PRINT == 1)

          f2 = fopen(oracle_responses_now_file_name, "w+");
          fclose(f2);

          f2 = fopen(valid_ct_file_now, "w+");
          fclose(f2);

          f2 = fopen(ct_file_now_basic, "w+");
          fclose(f2);

          f2 = fopen(ct_file_now, "w+");
          fclose(f2);

          f2 = fopen(keypair_file_now, "w+");
          fclose(f2);

          f2 = fopen(ct_file_basic_failed, "w+");
          fclose(f2);

        #endif

        printf("************************************************************************************************************\n");


        randombytes_init(seed[i], NULL, 256);

        // Generate new key pair ...

        if ( (ret_val = crypto_kem_keypair(pk, sk)) != 0)
        {
            return KAT_CRYPTO_FAILURE;
        }

          #if (DO_PRINT == 1)

        f2 = fopen(keypair_file_now, "a");

        for(int pp1=0;pp1<crypto_kem_PUBLICKEYBYTES;pp1++)
        {
          fprintf(f2,"%02x", pk[pp1]);
        }

        for(int pp1=0;pp1<crypto_kem_SECRETKEYBYTES;pp1++)
        {
          fprintf(f2,"%02x", sk[pp1]);
        }
        fclose(f2);

        #endif

        int successful_attack_done = 0;

        int no_queries = 0;
        int match_success = 0;

        uint8_t oracle_responses[TRIALS_FOR_SHUFFLING][4*p];
        uint8_t shuffling_oracle_responses[TRIALS_FOR_SHUFFLING][4*p];

        int oracle_response_count = 0;

        int rejected = 0;

        int success_touch = 0;
        int now_response_count = 0;

        int reached = 0;

        int profile_trials = 0;



        printf("Count of Single Collisions: %d\n", no_single_collisions);
        printf("Count of Multiple Collisions: %d\n", no_multiple_collisions);
        printf("Count of False Negative Collisions: %d\n", no_false_negative_collisions);
        printf("Count of False Positive Collisions: %d\n", no_false_positive_collisions);
        printf("Count of Trials Overall: %d\n", total_profile_trials_overall);

        printf("Probability of Single Collisions: %f\n", ((float)no_single_collisions/total_profile_trials_overall));
        printf("Probability of Multiple Collisions: %f\n", ((float)no_multiple_collisions/total_profile_trials_overall));
        printf("Probability of False Negative Collisions: %f\n", ((float)no_false_negative_collisions/total_profile_trials_overall));
        printf("Probability of False Positive Collisions: %f\n", ((float)no_false_positive_collisions/total_profile_trials_overall));



        // Iterate till you get the correct keys...


        while(successful_attack_done < 1)
        {

          // The reached variable basically tells if you have finished the attack phase...
          // If reached = 1 and got_secret == 0, then it means key recovery has failed...So, we need to try again...


          if(reached == 1 && success_touch == 0)
          {
            oracle_response_count = oracle_response_count - 4*p;
            match_success = 0;

            printf("Deleting...\n");

            #if (DO_PRINT == 1)

            f2 = fopen(oracle_responses_now_file_name, "w+");
            fclose(f2);

            f2 = fopen(valid_ct_file_now, "w+");
            fclose(f2);

            f2 = fopen(ct_file_now_basic, "w+");
            fclose(f2);

            f2 = fopen(ct_file_now, "w+");
            fclose(f2);

            f2 = fopen(ct_file_basic_failed, "w+");
            fclose(f2);

            #endif

          }

          match_success = 0;

          success_touch = 0;
          now_response_count = 0;

          rej:

          // The rejected variable tells whether the attack phase was aborted half way, maybe due to bad oracle responses.... Bad oracle responses are possible when
          // the base ciphertext is wrong...

          if(rejected == 1)
          {
            oracle_response_count = oracle_response_count - now_response_count;
            now_response_count = 0;
            rejected = 0;
            match_success = 0;

            printf("Deleting...\n");

            #if (DO_PRINT == 1)

            f2 = fopen(oracle_responses_now_file_name, "w+");
            fclose(f2);

            f2 = fopen(valid_ct_file_now, "w+");
            fclose(f2);

            f2 = fopen(ct_file_now_basic, "w+");
            fclose(f2);

            f2 = fopen(ct_file_now, "w+");
            fclose(f2);

            f2 = fopen(ct_file_basic_failed, "w+");
            fclose(f2);

            #endif


          }


          int success_trial = 0;

          int no_of_times_arrived = 0;

          while(success_trial == 0)
          {
            printf("Try: %d\n",profile_trials);

            profile_trials = profile_trials+1;
            total_profile_trials_overall = total_profile_trials_overall+1;

            // Here, we generate valid ciphertexts and decapsulate it...

            intended_function = 3;
            crypto_kem_enc(ct, ss, pk);
            crypto_kem_dec(ss1, ct, sk);

            f2 = fopen(valid_ct_file_now, "w+");

            for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
            {
              fprintf(f2,"%02x", ct[pp1]);
            }
            fclose(f2);

            int got_minus_one = 0;


            // Here, we generate our perturbation ciphertext... similar to the ciphertext for the PC oracle attack.... c = k1 . d1 + k2. d2. h

            intended_function = 0;

            crypto_kem_enc(ct, ss, pk);
            crypto_kem_dec(ss1, ct, sk);

            #if (COLL_CHECK == 1)

            // This is used for debug information, to see if the ciphertext resulted in a collision...This is only used to track the progress of the attack...No additional information is fed to perform the attack...

            for(int i = 0; i<count_threshold; i++)
            {
              collision_array_index[i] = 0;
            }

            for(int i = 0; i<count_threshold; i++)
            {
              collision_array_value[i] = 0;
            }

            for(int i = 0; i < p; i++)
            {
              if(er_decrypt[i] == 1 || er_decrypt[i] == -1)
                printf("e[%d]: %d\n", i, er_decrypt[i]);
            }

            count_minus_ones = 0;
            for(i = 0; i < p; i++)
            {
              hw_value = hw_calc(er_decrypt[i]);
              if(hw_value == 8 || hw_value == 1)
              {
                count_minus_ones = count_minus_ones+1;

              }
            }

            if(count_minus_ones > 0)
            {

              int index_ones = 0;
              for(int i = 0; i < p; i++)
              {
                if(er_decrypt[i] == 1 || er_decrypt[i] == -1)
                {
                  // printf("Collision Yes....\n");
                  collision_array_index[index_ones] = i;
                  collision_array_value[index_ones] = er_decrypt[i];
                  break;
                }
              }
            }

            #endif

            // Here, we add the perturbation to the valid ciphertext and decapsulate it...and observe if it results in a decryption failure...

            intended_function = 4;
            crypto_kem_enc(ct, ss, pk);
            crypto_kem_dec(ss1, ct, sk);

            // The decryption failure information is stored in the global_mask variable...

            printf("No True Collisions: %d, global_mask = %d\n", no_true_collisions, global_mask);
            if(no_true_collisions == 1 && global_mask == -1)
            {
              // Best case...
              no_single_collisions = no_single_collisions + 1;
              #if DO_ATTACK == 0
                got_minus_one = 1;
                printf("Single Collision...\n");
              #endif
            }
            if(no_true_collisions > 1 && global_mask == -1)
            {
              // Multiple collisions...
              printf("Multiple Collisions...\n");
              no_multiple_collisions = no_multiple_collisions + 1;
            }
            if(no_true_collisions == 1 && global_mask == 0)
            {
              // False Negative Collision
              printf("False Negative Collision...\n");
              no_false_negative_collisions = no_false_negative_collisions + 1;
            }
            if(no_true_collisions == 0 && global_mask == -1)
            {
              // False Positive Collision
              printf("False Positive Collisions...\n");
              no_false_positive_collisions = no_false_positive_collisions + 1;
            }



            if(global_mask == -1)
            {
              printf("got it...\n");

              #if DO_ATTACK == 1
              got_minus_one = got_minus_one+1;
              #endif

            }

            if(got_minus_one == 1)
            {
              success_trial = 1;

              #if (DO_PRINT == 1)

              f2 = fopen(ct_file_now_basic, "a");
              for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
              {
                fprintf(f2,"%02x", ct[pp1]);
              }
              fclose(f2);

              #endif

            }
            else
            {

              #if (DO_PRINT == 1)

              f2 = fopen(ct_file_basic_failed, "a");
              for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
              {
                fprintf(f2,"%02x", ct[pp1]);
              }
              fclose(f2);

              #endif

            }


          }


          // We have now got the base ciphertext... Now, we can do the attack phase...

          #if (COLL_CHECK == 1)

          collision_index = collision_array_index[0];
          collision_value = collision_array_value[0];

          printf("collision_index:%d,collision_value:%d\n", collision_index,collision_value);

          #endif

          int get_er_decrypt_array[2];

          int get_er_decrypt_array_all[4*p];
          int count_get_er_decrypt_array_all = 0;

          int check_value_whether_correct = 0;
          int no_coeffs_gone = 0;

          int found_secret_coeff;
          int final_secret_coeffs[p];

          sec_index = 0;

          int zero_indication = 0;
          int finding_secret_coeff = 0;
          int success_rate = 0;


          #if DO_ATTACK == 1

          // Here, we try to extract the oracle responses for c = cvalid + l1 . d1 + l2 . d2 . h + l3

          // With this, we can get the oracle reponse for the ith coefficient of the secret polynomial f...

          int trying_for_now = 0;
          for(int check1 = 0; check1 < 2; check1++)
          {
            int mul_value;

            // Whether to use c = (cvalid + l1 . d1 + l2 . d2 . h + l3) or  (cvalid + l1 . d1 + l2 . d2 . h - l3)

            if(check1 == 0)
              mul_value = 1;
            else
              mul_value = -1;

            // This is to indicate utilization of (l1, l2, l3) used to distinguish +1...

            check_for_value = mul_value * 1;

            // Here, we are querying the device with attack ciphertexts, built based on the base ciphertext and the valid ciphertext...

            intended_function = 1;
            crypto_kem_enc(ct, ss, pk);
            crypto_kem_dec(ss1, ct, sk);
            no_queries = no_queries+1;

            // Storing oracle response in get_er_decrypt_array...

            if(global_mask == -1)
              get_er_decrypt_array[0] = -1;
            else
              get_er_decrypt_array[0] = 0;

              // Storing oracle response also in get_er_decrypt_array_all... This array is used for collecting all oracle responses for all coefficients...

            if(global_mask == -1)
              get_er_decrypt_array_all[count_get_er_decrypt_array_all] = -1;
            else
              get_er_decrypt_array_all[count_get_er_decrypt_array_all] = 0;

            count_get_er_decrypt_array_all = count_get_er_decrypt_array_all + 1;

            #if (DO_PRINT == 1)

            // Writing attack ciphertext to text file...

            f2 = fopen(ct_file_now, "a");
            for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
            {
              fprintf(f2,"%02x", ct[pp1]);
            }
            fclose(f2);

            temp_temp_char = get_er_decrypt_array[0] & 0xFF;

            // Writing oracle response to text file...

            f2 = fopen(oracle_responses_now_file_name, "a");
            fprintf(f2,"%02x", temp_temp_char);
            fclose(f2);

            #endif

            // This is to indicate utilization of (l1, l2, l3) used to distinguish +2...

            check_for_value = mul_value * 2;

            // Repeating the same process...

            intended_function = 1;
            crypto_kem_enc(ct, ss, pk);
            crypto_kem_dec(ss1, ct, sk);
            no_queries = no_queries+1;

            if(global_mask == -1)
              get_er_decrypt_array[1] = -1;
            else
              get_er_decrypt_array[1] = 0;

            if(global_mask == -1)
              get_er_decrypt_array_all[count_get_er_decrypt_array_all] = -1;
            else
              get_er_decrypt_array_all[count_get_er_decrypt_array_all] = 0;

            count_get_er_decrypt_array_all = count_get_er_decrypt_array_all + 1;


            #if (DO_PRINT == 1)

            f2 = fopen(ct_file_now, "a");
            for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
            {
              fprintf(f2,"%02x", ct[pp1]);
            }
            fclose(f2);

            temp_temp_char = get_er_decrypt_array[1] & 0xFF;

            f2 = fopen(oracle_responses_now_file_name, "a");
            fprintf(f2,"%02x", temp_temp_char);
            fclose(f2);

            #endif

            // Here, we are simply checking the oracle responses, whether it agrees with the distinguisher table...If not, then simply discard and start the attack again....

            if(get_er_decrypt_array[0] == 0 && get_er_decrypt_array[1] == 0)
            {
              zero_indication = zero_indication + 1;
            }
            else if(get_er_decrypt_array[0] == -1 && get_er_decrypt_array[1] == 0)
            {

              if(zero_indication == 0 && check1 == 0)
              {
                #if (COLL_CHECK == 1)
                found_secret_coeff = -1*collision_value;
                #endif
              }
              else if(zero_indication == 1)
              {
                #if (COLL_CHECK == 1)
                found_secret_coeff = 1*collision_value;
                #endif
              }
              else
              {
                // Here, oracle response does not agree to the table...discard...

                rejected = 1;
                printf("Oracle Values...\n");
                goto rej;
              }

              finding_secret_coeff = 1;
            }
            else if(get_er_decrypt_array[0] == -1 && get_er_decrypt_array[1] == -1)
            {
              if(zero_indication == 0 && check1 == 0)
              {
                #if (COLL_CHECK == 1)
                found_secret_coeff = -2*collision_value;
                #endif
              }
              else if(zero_indication == 1)
              {
                #if (COLL_CHECK == 1)
                found_secret_coeff = 2*collision_value;
                #endif
              }
              else
              {
                // Here, oracle response does not agree to the table...discard...

                rejected = 1;
                printf("Oracle Values...\n");
                goto rej;
              }

              finding_secret_coeff = 1;
            }
            else
            {
              // Here, oracle response does not agree to the table...discard...

              rejected = 1;
              printf("Oracle Values...\n");
              goto rej;
            }

          }

          if((finding_secret_coeff == 0) && (zero_indication == 2))
          {
            found_secret_coeff = 0;
            no_queries = no_queries - 1;
          }
          else if(finding_secret_coeff == 0)
          {
            // Here, oracle response does not agree to the table...discard...

            rejected = 1;
            printf("Unreliable Values...\n");
            goto rej;
          }

          #if (COLL_CHECK == 1)

          final_secret_coeffs[collision_index] = found_secret_coeff;

          if(found_secret_coeff == global_f[collision_index])
          {
            success_rate = success_rate + 1;
          }

          #endif

          int touch_all = 0;
          int touch_0 = 0;
          int touch_1 = 0;
          int touch_2 = 0;

          // Here, we try to extract the oracle responses for ciphertexts formed as
          // c = (cvalid + l1 . d1 + l2 . d2 . h + l3 . x^u) where u is swept from p-1 until 1...

          for(sec_index = p-1; sec_index >= 1; sec_index--)
          {

            int coeff_now, coeff_now_1, coeff_now_2;
            int f_coeff_value;

            #if (COLL_CHECK == 1)

            // In the debug mode, we know the collision index and the collision value... so, we use that to track the actual secret coefficient that
            // corresponds to the current iteration... So, we use this information to track the success of key recovery using the
            // oracle's responses..... But, this information is not used to perform/aid the attack...We simply use it for monitoring
            // purposes...
            // The correct secret coefficient is stord in the f_coeff_value variable...
            // Global_f stores the value of the secret polynomiai f...

            int which_one;

            if(collision_index > sec_index)
            {
              coeff_now = collision_index-sec_index;
              f_coeff_value = global_f[coeff_now];
              which_one = 1;
            }
            else if(collision_index == 0)
            {
              coeff_now = p - sec_index;
              f_coeff_value = global_f[coeff_now];
              which_one = 1;
            }
            else if(collision_index <= sec_index && collision_index > 0)
            {
              coeff_now_1 = (p - sec_index + collision_index - 1)%(p);
              coeff_now_2 = (p - sec_index + collision_index)%(p);
              f_coeff_value = global_f[coeff_now_1] + global_f[coeff_now_2];
              which_one = 2;
            }

            #endif

            int zero_indication = 0;
            int finding_secret_coeff = 0;

            // Here, we are collecting oracle responses in a very similar manner...

            for(int check1 = 0; check1 < 2; check1++)
            {
              int mul_value;

              if(check1 == 0)
                mul_value = 1;
              else
                mul_value = -1;

              check_for_value = mul_value * 1;

              intended_function = 1;
              crypto_kem_enc(ct, ss, pk);
              crypto_kem_dec(ss1, ct, sk);
              no_queries = no_queries+1;



              if(global_mask == -1)
                get_er_decrypt_array[0] = -1;
              else
                get_er_decrypt_array[0] = 0;


              if(global_mask == -1)
                get_er_decrypt_array_all[count_get_er_decrypt_array_all] = -1;
              else
                get_er_decrypt_array_all[count_get_er_decrypt_array_all] = 0;

              count_get_er_decrypt_array_all = count_get_er_decrypt_array_all + 1;


              #if (DO_PRINT == 1)

              f2 = fopen(ct_file_now, "a");
              for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
              {
                fprintf(f2,"%02x", ct[pp1]);
              }
              fclose(f2);

              temp_temp_char = get_er_decrypt_array[0] & 0xFF;

              f2 = fopen(oracle_responses_now_file_name, "a");
              fprintf(f2,"%02x", temp_temp_char);
              fclose(f2);

              #endif

              check_for_value = mul_value * 2;

              intended_function = 1;
              crypto_kem_enc(ct, ss, pk);
              crypto_kem_dec(ss1, ct, sk);
              no_queries = no_queries+1;

              if(global_mask == -1)
                get_er_decrypt_array[1] = -1;
              else
                get_er_decrypt_array[1] = 0;


              if(global_mask == -1)
                get_er_decrypt_array_all[count_get_er_decrypt_array_all] = -1;
              else
                get_er_decrypt_array_all[count_get_er_decrypt_array_all] = 0;

              count_get_er_decrypt_array_all = count_get_er_decrypt_array_all + 1;

              #if (DO_PRINT == 1)

                f2 = fopen(ct_file_now, "a");
                for(int pp1=0;pp1<crypto_kem_CIPHERTEXTBYTES;pp1++)
                {
                  fprintf(f2,"%02x", ct[pp1]);
                }
                fclose(f2);

                temp_temp_char = get_er_decrypt_array[1] & 0xFF;

                f2 = fopen(oracle_responses_now_file_name, "a");
                fprintf(f2,"%02x", temp_temp_char);
                fclose(f2);

              #endif

              // This is to see if there are too many zero guesses... To see if the oracle's responses are skewed...This also indicates that the key
              // recovery is not proceeding correctly...

              check_value_whether_correct = check_value_whether_correct + ((get_er_decrypt_array[0] ^ get_er_decrypt_array[1]) & 0x1);

              // Here, we are analyzing the oracle's response....

              if(get_er_decrypt_array[0] == 0 && get_er_decrypt_array[1] == 0)
              {
                zero_indication = zero_indication + 1;
                touch_0 += 1;
              }
              else if(get_er_decrypt_array[0] == -1 && get_er_decrypt_array[1] == 0)
              {
                touch_1 += 1;
                if(zero_indication == 0 && check1 == 0)
                {
                  #if (COLL_CHECK == 1)
                  found_secret_coeff = -1*collision_value;
                  #endif
                }
                else if(zero_indication == 1)
                {
                  #if (COLL_CHECK == 1)
                  found_secret_coeff = collision_value;
                  #endif
                }
                else
                {
                  // Bad oracle responses....
                  rejected = 1;
                  printf("Oracle Values...\n");
                  goto rej;
                }

                finding_secret_coeff = 1;
              }
              else if(get_er_decrypt_array[0] == -1 && get_er_decrypt_array[1] == -1)
              {
                touch_2 += 1;
                if(zero_indication == 0 && check1 == 0)
                {
                  #if (COLL_CHECK == 1)
                  found_secret_coeff = -2*collision_value;
                  #endif
                }
                else if(zero_indication == 1)
                {
                  #if (COLL_CHECK == 1)
                  found_secret_coeff = 2*collision_value;
                  #endif
                }
                else
                {
                  // Bad oracle responses....

                  rejected = 1;
                  printf("Oracle Values...\n");
                  goto rej;
                }

                finding_secret_coeff = 1;
              }
              else
              {
                // Bad oracle responses....

                rejected = 1;
                printf("Oracle Values...\n");
                goto rej;
              }
            }

            if((finding_secret_coeff == 0) && (zero_indication == 2))
            {
              found_secret_coeff = 0;
              no_queries = no_queries - 1;
            }
            else if(finding_secret_coeff == 0)
            {
              // Bad oracle responses....

              rejected = 1;
              printf("Unreliable Values...\n");
              goto rej;
            }

            #if (COLL_CHECK == 1)

            if(which_one == 2)
            {
              final_secret_coeffs[(p - sec_index + collision_index)%p] = found_secret_coeff - final_secret_coeffs[(p - sec_index + collision_index - 1)%p];

              if(final_secret_coeffs[(p - sec_index + collision_index)%p] == global_f[(p - sec_index + collision_index)%p])
              {
                success_rate = success_rate + 1;
              }

            }
            if(which_one == 1)
            {
              final_secret_coeffs[coeff_now] = found_secret_coeff;

              if(final_secret_coeffs[coeff_now] == global_f[coeff_now])
              {
                success_rate = success_rate + 1;
              }

            }

            printf("Success: %d/%d\n",success_rate,p-sec_index+1);

            #endif

            // Here, we are keeping track of the number of +/-1 , +/- 2 and 0 coefficients and if they seem to be skewed.... Then, you can reject and start the attakc again...
            // The number of iterations after which you want to check is upto you...We choose 100 here... and the expected distribution of the values also can be chosen by you...

            if(no_coeffs_gone == 100)
            {
              printf("touch_0: %d, touch_1: %d, touch_2: %d\n", touch_0, touch_1, touch_2);
              if((check_value_whether_correct < 10) || (touch_0 < 10) || (touch_1 < 5) || (touch_2 < 2) || (abs(touch_1-touch_0)) < 30 || (touch_1 > touch_0))
              {
                rejected = 1;
                printf("Only zeros guessed...\n");
                goto rej;
              }
            }

            no_coeffs_gone = no_coeffs_gone + 1;
          }

          // Here, we have finished collecting oracle respones for all coefficients... also evading all the conditional checks for abortion...
          // To recover the correct secret key, we need the collision value as well as the collision index...but attacker does not know both...
          // So, we try all possible values of the collision value (+/- 2) and also the collision coefficient from 0 to p-1...

          int break_break = 0;

          // Iterating over all collision indices...
          for(int coll_index = p-1; coll_index >= 0; coll_index--)
          {
            // Iterating over all collision values...
            for(int coll_value = 0; coll_value  <= 1; coll_value++)
            {

              int actual_coll_value = 0;
              if(coll_value == 0)
                actual_coll_value = 1;
              else
                actual_coll_value = -1;

              // Implementing the distinguisher table... for the first coefficient... This information is then used to recover the other coefficients...

              if(get_er_decrypt_array_all[0] == 0 && get_er_decrypt_array_all[1] == 0 && get_er_decrypt_array_all[2] == 0 && get_er_decrypt_array_all[3] == 0)
              {
                final_secret_coeffs[coll_index] = 0;
              }
              else if(get_er_decrypt_array_all[0] == -1 && get_er_decrypt_array_all[1] == 0 && get_er_decrypt_array_all[2] == 0 && get_er_decrypt_array_all[3] == 0)
              {
                final_secret_coeffs[coll_index] = -1*actual_coll_value;
              }
              else if(get_er_decrypt_array_all[0] == 0 && get_er_decrypt_array_all[1] == 0 && get_er_decrypt_array_all[2] == -1 && get_er_decrypt_array_all[3] == 0)
              {
                final_secret_coeffs[coll_index] = 1*actual_coll_value;
              }
              else if(get_er_decrypt_array_all[0] == -1 && get_er_decrypt_array_all[1] == -1 && get_er_decrypt_array_all[2] == 0 && get_er_decrypt_array_all[3] == 0)
              {
                final_secret_coeffs[coll_index] = -2*actual_coll_value;
              }
              else if(get_er_decrypt_array_all[0] == 0 && get_er_decrypt_array_all[1] == 0 && get_er_decrypt_array_all[2] == -1 && get_er_decrypt_array_all[3] == -1)
              {
                final_secret_coeffs[coll_index] = 2*actual_coll_value;
              }

              int which_one;
              int coeff_now;
              int f_coeff_value;
              int coeff_now_1, coeff_now_2;
              int found_secret_coeff;

              // Trying to analyze oracle response and getting the secret coefficients...

              for(int sec_index = p-1; sec_index >= 1; sec_index--)
              {

                if(coll_index > sec_index)
                {
                  coeff_now = coll_index-sec_index;
                  f_coeff_value = global_f[coeff_now];
                  which_one = 1;
                }
                else if(coll_index == 0)
                {
                  coeff_now = p - sec_index;
                  f_coeff_value = global_f[coeff_now];
                  which_one = 1;
                }
                else if(coll_index <= sec_index && coll_index > 0)
                {
                  coeff_now_1 = (p - sec_index + coll_index - 1)%(p);
                  coeff_now_2 = (p - sec_index + coll_index)%(p);
                  f_coeff_value = global_f[coeff_now_1] + global_f[coeff_now_2];
                  which_one = 2;
                }

                // Implementing the distinguisher table...

                if(get_er_decrypt_array_all[((p-1)-sec_index+1)*4] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+1] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+2] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+3] == 0)
                {
                  found_secret_coeff = 0;
                }
                else if(get_er_decrypt_array_all[((p-1)-sec_index+1)*4] == -1 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+1] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+2] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+3] == 0)
                {
                  found_secret_coeff = -1*actual_coll_value;
                }
                else if(get_er_decrypt_array_all[((p-1)-sec_index+1)*4] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+1] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+2] == -1 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+3] == 0)
                {
                  found_secret_coeff = 1*actual_coll_value;
                }
                else if(get_er_decrypt_array_all[((p-1)-sec_index+1)*4] == -1 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+1] == -1 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+2] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+3] == 0)
                {
                  found_secret_coeff = -2*actual_coll_value;
                }
                else if(get_er_decrypt_array_all[((p-1)-sec_index+1)*4] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+1] == 0 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+2] == -1 && get_er_decrypt_array_all[((p-1)-sec_index+1)*4+3] == -1)
                {
                  found_secret_coeff = 2*actual_coll_value;
                }

                if(which_one == 2)
                {
                  final_secret_coeffs[(p - sec_index + coll_index)%p] = found_secret_coeff - final_secret_coeffs[(p - sec_index + coll_index - 1)%p];
                }

                if(which_one == 1)
                {
                  final_secret_coeffs[coeff_now] = found_secret_coeff;
                }
              }

              int check_value = 0;
              int weight_fff = 0;

              // Now, we check whether the recovered coefficients have a weight of w... according to the parameter set...

              for(int qw = 0; qw < p; qw++)
              {
                weight_fff = weight_fff + abs(final_secret_coeffs[qw]);

                if(abs(final_secret_coeffs[qw]) > 1)
                {
                  check_value = 1;
                  break;
                }
              }

              // If so, then we have succeeded... Typically you have multiple choices for the secret polynomial... approximately 5 to 10...

              if(check_value == 0 && weight_fff == w)
              {

                printf("*************************************************************************************\n");
                printf("Secret Coefficient for coll index: %d, coll_value: %d\n",coll_index,actual_coll_value);

                for(int hfh = 0; hfh < p; hfh++)
                {
                  printf("%d, ",final_secret_coeffs[hfh]);
                }
                printf("\n");


                weight_for_secret = 0;

                success_rate = 0;
                for(int rf = 0; rf < p; rf++)
                {
                  weight_for_secret = weight_for_secret + abs(final_secret_coeffs[rf]);
                  if(final_secret_coeffs[rf] == global_f[rf])
                    success_rate = success_rate+1;
                }

                if(success_rate != p)
                {
                  success_rate = 0;
                  for(int rf = 0; rf < p; rf++)
                  {
                    if(final_secret_coeffs[rf] == (-1*global_f[rf]))
                      success_rate = success_rate+1;
                  }
                }

                if(success_rate == p)
                {

                  success_touch = 1;
                  successful_attack_done = successful_attack_done+1;
                  printf("Success... Correct key recovered...\n");

                  printf("Success: %d/%d\n",success_rate,p);
                  printf("No of Queries: %d\n",no_queries + (profile_trials)*10);

                }

              }

            }

          }

          reached = 1;

          profile_average_count = profile_average_count + profile_trials;
          trace_average_count = trace_average_count + (no_queries + (profile_trials)*10);

          printf("Profile Aveage: %fn",profile_average_count/(pq+1));
          printf("Trace Average: %f\n",trace_average_count/(pq+1));

          #else
            successful_attack_done = successful_attack_done+1;
          #endif

        }
    }

    profile_average_count = profile_average_count/NO_TESTS;
    trace_average_count = trace_average_count/NO_TESTS;

    printf("profile_average: %f\n",profile_average_count);
    printf("trace_average: %f\n",trace_average_count);

    return KAT_SUCCESS;
}
