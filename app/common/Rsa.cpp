/*
 * Rsa.cpp
 *
 *  Created on: 2011-9-27
 *      Author: dada
 */

#include "Common.h"

//#include <openssl/evp.h>
//#include <openssl/x509.h>
//#include <openssl/x509v3.h>
//#include <openssl/ssl.h>

///TODO: NOT COMPLETE

bool Crypt::RsaSha1Signature(const string &text, const string &key, string &sign)
{
	return false;
//	unsigned char *sig = NULL;
//	unsigned char *passphrase = NULL;
//	unsigned int len=0;
//	EVP_MD_CTX md_ctx;
//
//	EVP_PKEY *pkey;
//	BIO *in;
//	in = BIO_new_mem_buf((void *)const_cast<char *>(key.c_str()), key.size());
//	pkey = PEM_read_bio_PrivateKey(in, NULL, 0, passphrase); // generate sign
//	BIO_free(in);
//
//	if(pkey == NULL)
//	{
//		return false;
//	}
//
//	len = EVP_PKEY_size(pkey);
//	sig = new unsigned char[(len+1)*sizeof(char)];
//
//	EVP_SignInit(&md_ctx, EVP_sha1());
//	EVP_SignUpdate(&md_ctx, text.c_str(), text.size());
//	if (!EVP_SignFinal (&md_ctx, sig, &len, pkey))
//	{
//		delete sig;
//		return false;
//	}
//
//	sign.clear();
//	sign.append((char *)sig, len);
//	delete sig;
//	OPENSSL_free(sig);
//	EVP_PKEY_free(pkey);
//	return true;
}

bool Crypt::RsaSha1Verify(const string &text, const string &key, const string &sign)
{
	return false;
//	EVP_MD_CTX md_ctx;
//	EVP_PKEY *pkey;
//	BIO *in;
//	X509 *cert = NULL;
//	int err;
//
//	in = BIO_new_mem_buf((void *)const_cast<char *>(key.c_str()), key.size());
//	cert = PEM_read_bio_X509(in, NULL, 0, NULL);
//	if (cert)
//	{
//		pkey = (EVP_PKEY *) X509_get_pubkey(cert);
//		X509_free(cert);
//	}
//	else
//	{
//		pkey = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);
//	}
//	BIO_free(in);
//	if (pkey == NULL)
//	{
//		return false;
//	}
//
//	EVP_VerifyInit(&md_ctx, EVP_sha1());
//	EVP_VerifyUpdate(&md_ctx, text.c_str(), text.size());
//	err = EVP_VerifyFinal(&md_ctx, (const unsigned char *)sign.c_str(), sign.size(), pkey);
//	EVP_MD_CTX_cleanup(&md_ctx);
//	EVP_PKEY_free(pkey);
//	return err == 1;
}


