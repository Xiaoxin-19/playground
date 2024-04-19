#include <l8w8jwt/encode.h>
#include <l8w8jwt/decode.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
  unsigned long user_id;
  char workpath[256];
} token_info;

int generate_token(token_info *info, char *JWT);
int verify_token(const char *JWT, token_info *info);
char KEY[] = "test_key";
char iss[] = "NetDisk";
int main(void)
{
  token_info info;
  info.user_id = 123456;
  strcpy(info.workpath, "/home/username/NetDisk/");
  char jwt[1024];
  generate_token(&info, jwt);
  printf("jwt: %s", jwt);
  verify_token(jwt, &info);
  return 0;
}

int generate_token(token_info *info, char *JWT)
{
  size_t jwt_length;

  struct l8w8jwt_claim payload_claims[] =
      {
          {.key = "workpath",
           .key_length = 8,
           .value = info->workpath,
           .value_length = strlen(info->workpath),
           .type = L8W8JWT_CLAIM_TYPE_STRING},
      };

  struct l8w8jwt_encoding_params params;
  l8w8jwt_encoding_params_init(&params);

  params.alg = L8W8JWT_ALG_HS512;
  char user_id[65] = {0};
  sprintf(user_id, "%ld", info->user_id);
  params.sub = user_id;
  params.sub_length = strlen(user_id);

  params.iss = iss;
  params.iss_length = strlen(iss);

  params.iat = l8w8jwt_time(NULL);
  params.exp = l8w8jwt_time(NULL) + 600; // Set to expire after 10 minutes (600 seconds).

  params.additional_payload_claims = payload_claims;
  params.additional_payload_claims_count = sizeof(payload_claims) / sizeof(struct l8w8jwt_claim);

  params.secret_key = (char *)KEY;
  params.secret_key_length = strlen(KEY);
  char *tmep_jwt = (char *)malloc(1024);
  params.out = &tmep_jwt;
  params.out_length = &jwt_length;

  int r = l8w8jwt_encode(&params);
  strcpy(JWT, tmep_jwt);
  printf("\nl8w8jwt_encode_hs512 function returned %s (code %d).\n\nCreated token: \n%s\n", r == L8W8JWT_SUCCESS ? "successfully" : "", r, JWT);
}

int verify_token(const char *JWT, token_info *info)
{
  struct l8w8jwt_decoding_params params;
  l8w8jwt_decoding_params_init(&params);

  params.alg = L8W8JWT_ALG_HS512;

  printf("JWT: %s\n", JWT);
  params.jwt = (char *)JWT;
  params.jwt_length = strlen(JWT);
  params.verification_key = (unsigned char *)KEY;
  params.verification_key_length = strlen(KEY);

  struct l8w8jwt_claim *claims = NULL;
  size_t claim_count = 0;

  enum l8w8jwt_validation_result validation_result;
  int r = l8w8jwt_decode(&params, &validation_result, &claims, &claim_count);
  for (size_t i = 0; i < claim_count; i++)
  {
    printf("Claim [%zu]: %s = %s\n", i, claims[i].key, claims[i].value);
  }

  printf("\nl8w8jwt_decode_hs512 function returned %s (code %d).\n\nValidation result: \n%d\n", r == L8W8JWT_SUCCESS ? "successfully" : "", r, validation_result);
}
