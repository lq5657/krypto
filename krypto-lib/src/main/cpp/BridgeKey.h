/*Accessed by client, used to generate components of the public keys*/
#ifndef krypto_BridgeKey_h
#define krypto_BridgeKey_h

#include <assert.h>
#include "PrivateKey.h"
#include "BitMatrix.h"

using namespace std;

//TODO: make BridgeKey a friend class of PrivateKey

template<unsigned int N, unsigned int L>
class BridgeKey{
public:
	BridgeKey(PrivateKey<N,L> pk, BitMatrix<N> K) : 
	_pk(pk),
	_R(BitMatrix<N>::randomInvertibleMatrix()),
	_Rx(BitMatrix<N>::randomInvertibleMatrix()),
	_Ry(BitMatrix<N>::randomInvertibleMatrix()),
	_M(pk.getM()),
	_Cu1(pk.getUnaryObfChain()[1]),
	_Cu2(pk.getUnaryObfChain()[2]),
	_Cb1(pk.getBinaryObfChain()[1]),
	_Cb2(pk.getBinaryObfChain()[2]),
	_BKBi(pk.getB() * K * pk.getB().inv()),
	_BKBiAi(_BKBi * pk.getA().inv()),
	_ARAi(pk.getA() * _R * pk.getA().inv()),
	_ARxAi(pk.getA() * _Rx * pk.getA().inv()),
	_ARyAi(pk.getA() * _Ry * pk.getA().inv())
	{
	}	

/* Left Matrix Multiplication */

	const BitMatrix<2*N> get_LMM_Z() const{
		//untested!
		BitMatrix<N> zeroN = BitMatrix<N>::squareZeroMatrix();

		BitMatrix<2*N> X_top = BitMatrix<N>::aug_h(_BKBi, _BKBiAi);
		BitMatrix<2*N> X_bottom = BitMatrix<N>::aug_h(zeroN, _ARAi);
		BitMatrix<2*N> X = BitMatrix<2*N>::aug_v(X_top, X_bottom) * _M.inv();

		BitMatrix<2*N> Y_top = BitMatrix<N>::aug_h(_BKBi, BitMatrix<N>::squareIdentityMatrix());
		BitMatrix<2*N> Y_bottom = BitMatrix<N>::aug_h(zeroN, zeroN);
		BitMatrix<2*N> Y = BitMatrix<2*N>::aug_v(X_top, X_bottom) * _Cu2.inv();
		return BitMatrix<2*N>::aug_h(X, Y);
	}

	const BitMatrix<N> get_LMM_g1() const{
		//to be implemented
		return BitMatrix<N>::randomInvertibleMatrix();
	}

	const BitMatrix<N> get_LMM_g2() const{
		//to be implemented
		return BitMatrix<N>::randomInvertibleMatrix();
	}


/* XOR */

	const BitMatrix<2*N> get_XOR_Xx() const{
		//untested!
		BitMatrix<N> idN = BitMatrix<N>::squareIdentityMatrix();

		BitMatrix<2*N> X_top = BitMatrix<N>::aug_h(idN, (idN + _Rx) * _pk.getA.inv());
		BitMatrix<2*N> X_bottom = BitMatrix<N>::aug_h(BitMatrix<N>::squareZeroMatrix(), _ARxAi);
		return BitMatrix<2*N>::aug_v(X_top, X_bottom) * _M.inv();
	}

	const BitMatrix<2*N> get_XOR_Xy() const{
		//untested!
		BitMatrix<N> idN = BitMatrix<N>::squareIdentityMatrix();

		BitMatrix<2*N> X_top = BitMatrix<N>::aug_h(idN, (idN + _Ry) * _pk.getA.inv());
		BitMatrix<2*N> X_bottom = BitMatrix<N>::aug_h(BitMatrix<N>::squareZeroMatrix(), _ARyAi);
		return BitMatrix<2*N>::aug_v(X_top, X_bottom) * _M.inv();
	}

	const BitMatrix<3*N> get_XOR_Y() const{
		//untested!
		BitMatrix<N> idN = BitMatrix<N>::squareIdentityMatrix();

		BitMatrix<3*N> Y_top = BitMatrix<N>::aug_h(idN, BitMatrix<N>::aug_h(idN, idN));
		return BitMatrix<3*N>::aug_v(Y_top, BitMatrix<3*N>::zeroMatrix(N << 6)) * _Cu2.inv();
	}

	const BitMatrix<3*N> get_XOR_g1() const{
		//to be implemented
		return BitMatrix<3*N>::randomInvertibleMatrix();
	}

	const BitMatrix<3*N> get_XOR_g2() const{
		//to be implemented
		return BitMatrix<3*N>::randomInvertibleMatrix();
	}


/* AND */

	const BitVector<2*N> get_AND_z() const{
		//to be implemented
		return BitVector<2*N>::randomVector();
	}

	const BitMatrix<2*N> get_AND_Z1() const{
		//untested!
		BitMatrix<2*N> M2 = _M.split_v(1, 2);
		BitMatrix<2*N> top = _Rx * _pk.getA().inv() * M2;
		BitMatrix<2*N> bottom = _pk.getA() * top;
		return BitMatrix<N>::aug_v(top, bottom);
	}

	const BitMatrix<N> get_AND_Z2() const{
		BitMatrix<2*N> M2 = _M.split_v(1, 2);
		BitMatrix<2*N> top = _Ry * _pk.getA().inv() * M2;
		BitMatrix<2*N> bottom = _pk.getA() * top;
		return BitMatrix<N>::aug_v(top, bottom);
	}

	const BitMatrix<N> get_AND_g1() const{
		//to be implemented
		return BitMatrix<N>::randomInvertibleMatrix();
	}

	const BitMatrix<N> get_AND_g2() const{
		//to be implemented
		return BitMatrix<N>::randomInvertibleMatrix();
	}


private:
	PrivateKey<N,L> _pk;
	BitMatrix<N> _R; //TODO: delegate the random matrix generation task to some other class?
	BitMatrix<N> _Rx;
	BitMatrix<N> _Ry;
	BitMatrix<N> _M;
	BitMatrix<2*N> _Cu1;
	BitMatrix<2*N> _Cu2;
	BitMatrix<3*N> _Cb1;
	BitMatrix<3*N> _Cb2;
	BitMatrix<N> _BKBi; 
	BitMatrix<N> _BKBiAi;
	BitMatrix<N> _ARAi;
	BitMatrix<N> _ARxAi;
	BitMatrix<N> _ARyAi;
	PolynomialFunctionTupleChain<2*N,L> _g_u; //obsfucated chain for unary operations
	PolynomialFunctionTupleChain<3*N,L> _g_b; //obsfucated chain for binary operations
	int _dim_quad = 64; //dimension of bitmatrix used to represent quadratic poly's


	//can be combined into get_z
	const BitMatrix<2*N> get_AND_X() const{
		BitMatrix<2*N> inner = BitMatrix<2*N>::aug_h(BitMatrix<2*N>::squareIdentityMatrix(), _pk.getA().inv());
		return _pk.getB().inv() * inner * _M.inv();
	}

	const BitMatrix<3*N> get_AND_Y1() const{
		BitMatrix<3*N> Cb_top = _Cb1.inv().split_v(1, 3);
		return _pk.getB() * Cb_top;
	}

	const BitMatrix<3*N> get_AND_Y2() const{
		BitMatrix<3*N> Cb_middle = _Cb1.inv().split_v(2, 3);
		return _pk.getB() * Cb_middle;
	}

	const BitMatrix<3*N> get_AND_Y3() const{
		return _Cb1.inv().split_v(3, 3);
	}

	//top chunk of contrib matrix for z
	//level ranges from 0 to 64 * 2N - 1
	const BitMatrix<N> get_AND_contrib_X_X_Y2(const int level, const BitMatrix<2*N> X, const BitMatrix<3*N> Y2) const{
		//untested!
		const int twoN = N << 7;
		const int threeN = 3 * (N << 6);
		BitMatrix<N> top = BitMatrix<N>::zeroMatrix(twoN - level); //TODO: check that ops with empty top matrix work

		BitMatrix<N> mid = BitMatrix<N>::zeroMatrix(twoN);
		for (int i = 0; i < twoN; i++) { //row within middle block
			for (int j = 0; j < (N << 6); j++) { //col within middle block
				bool lhs = X.get(j, level);
				bool rhs = X.get(j, i);
				mid.set(i, j, lhs && rhs);
			}
		}

		BitMatrix<N> bot = BitMatrix<N>::zeroMatrix(threeN);
		for (int i = 0; i < threeN; i++) { //row within bottom block
			for (int j = 0; j < (N << 6); j++) { //col within bottom block
				bool lhs = X.get(j, level);
				bool rhs = Y2.get(j, i);
				bot.set(i, j, lhs && rhs);
			}
		}
		return BitMatrix<N>::aug_v(BitMatrix<N>::aug_v(top, mid), bot);
	}

	//middle chunk of contrib matrix for z
	//level ranges from 0 to 64 * 2N - 1
	const BitMatrix<N> get_AND_contrib_X_Y1(const int level, const BitMatrix<2*N> X, const BitMatrix<3*N> Y1) const{
		//untested!
		const int twoN = N << 7;
		const int threeN = 3 * (N << 6);
		BitMatrix<N> top = BitMatrix<N>::zeroMatrix(twoN - level); //TODO: check that ops with empty top matrix work

		BitMatrix<N> bot = BitMatrix<N>::zeroMatrix(threeN);
		for (int i = 0; i < threeN; i++) { //row within bottom block
			for (int j = 0; j < (N << 6); j++) { //col within bottom block
				bool lhs = X.get(j, level);
				bool rhs = Y1.get(j, i);
				bot.set(i, j, lhs && rhs);
			}
		}
		return BitMatrix<N>::aug_v(top, bot);
	}

	//bottom chunk of contrib matrix for z
	//level ranges from 0 to 64 * 2N - 1
	const BitMatrix<N> get_AND_contrib_Y1_Y2(const int level, const BitMatrix<3*N> Y1, const BitMatrix<3*N> Y2) const{
		//to be implemented
		const int threeN = 3 * (N << 6); //should be the number of coefficients (___ choose 2)

		BitMatrix<N> contrib = BitMatrix<N>::zeroMatrix(threeN);
		return contrib;
	}
};

#endif