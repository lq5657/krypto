//
//  PrivateKey.h
//  krypto
//
//  Created by Peng Hui How and Robin Zhang on 7/22/15.
//  Copyright (c) 2015 Kryptnostic. All rights reserved.
//
//  Implementation of private key generation
//	Accessed by clients
//

#ifndef krypto_PrivateKey_h
#define krypto_PrivateKey_h

#include "MultiQuadTupleChain.h"

/*
 * Template for PrivateKey
 * Length of plaintext = N * 2^6, Length of obfuscation chain = L
 */
template<unsigned int N = 128>
class PrivateKey {

template<unsigned int N1> friend class BridgeKey;
template<unsigned int N2> friend class ClientHashFunction;
template<unsigned int N3> friend class SearchPrivateKey;
template<unsigned int N4> friend class KryptnosticClient;

public:

	/*
     * Default constructor
     * Constructs a PrivateKey with randomly initialized private variables
     */
	PrivateKey():
		_M(BitMatrix<2*N>::randomInvertibleMatrix()),
		_f(MultiQuadTupleChain<N,2>::randomMultiQuadTupleChain()){
		generateObfuscationMatrixChains();
	}

    /*
     * Function: encrypt(m)
     * Returns the encrypted ciphertext (length 2N * 2^6) of the plaintext (length N * 2^6)
     */
	const BitVector<2*N> encrypt(const BitVector<N> &m) const{//returns x = E(m, r) given a plaintext m
		const BitVector<N> & r = BitVector<N>::randomVector();
		const BitVector<N> & top = m ^ _f(r);
		const BitVector<N> & bottom = r;
		return _M * BitVector<N>::vCat(top, bottom);
	}

    /*
     * Function: decrypt(x)
     * Returns the decrypted plaintext (length N * 2^6) from a ciphertext (length 2N * 2^6)
     */
	const BitVector<N> decrypt(const BitVector<2*N> &x) const{//returns m = D(x) given a ciphertext x
		const BitVector<2*N> & mix = _M.solve(x);
		BitVector<N> x1, x2;
		mix.proj(x1, x2);
		const BitVector<N> & fx2 = _f(x2);
		return x1 ^ fx2;
	}

protected:
	const BitMatrix<2*N> getM() const{
		return _M;
	}

	const MultiQuadTupleChain<N,2> getf() const{
		return _f;
	}

	const BitMatrix<2*N> getUnaryObf1() const{
		return _Cu[0];
	}

	const BitMatrix<2*N> getUnaryObf2() const{
		return _Cu[1];
	}

	const BitMatrix<3*N> getBinaryObf1() const{
		return _Cb[0];
	}

	const BitMatrix<3*N> getBinaryObf2() const{
		return _Cb[1];
	}

private:
	const BitMatrix<2*N> _M; //SL_{2n}(F_2)
	MultiQuadTupleChain<N,2> _f; //{f_1,...,f_L} random quadratic function tuples
	BitMatrix<2*N> _Cu[2]; //chain of obfuscation matrix for unary operations
	BitMatrix<3*N> _Cb[2]; //chain of obfuscation matrix for binary operations

	void generateObfuscationMatrixChains(){ //generates C_{u1},...,C_{uL} and C_{b1},...,C_{bL}
		for(unsigned int i = 0; i < 2; ++i){
			_Cu[i] = BitMatrix<(2*N)>::randomInvertibleMatrix();
			_Cb[i] = BitMatrix<(3*N)>::randomInvertibleMatrix();
		}
	}
};

#endif /* defined(__krypto__PrivateKey__) */