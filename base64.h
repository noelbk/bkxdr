#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

#ifdef __cplusplus
#extern "C" {
#endif // __cplusplus


int
base64_enc(char *str, int len, char *out, int outlen);


int
base64_dec(char *str, int len, char *out, int outlen);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BASE64_H_INCLUDED
