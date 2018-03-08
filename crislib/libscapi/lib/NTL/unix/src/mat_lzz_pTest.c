
#include <NTL/mat_lzz_p.h>

NTL_CLIENT



void FillRandom(Mat<zz_p>& A)
{
   long n = A.NumRows();
   long m = A.NumCols();
   for (long i = 0; i < n; i++)
      for (long j = 0; j < m; j++)
         random(A[i][j]);
}

void FillRandom(Vec<zz_p>& A)
{
   long n = A.length();
   for (long i = 0; i < n; i++)
      random(A[i]);
}

int main(int argc, char **argv)
{

   cerr << "testing multiplication";
   for (long cnt = 0; cnt < 75; cnt++) {
      cerr << ".";
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(1000);
      long l = RandomBnd(1000);
      long m = RandomBnd(1000);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A, B, X;

      A.SetDims(n, l);
      B.SetDims(l, m);

      FillRandom(A);
      FillRandom(B);

      X.SetDims(n, m);

      vec_zz_p R;

      R.SetLength(m);
      for (long i = 0; i < m; i++) random(R[i]);

      mul(X, A, B);

      if (X*R != A*(B*R)) 
         cerr << "*\n*\n*\n*\n*\n*********** oops " << len << " " << n << " " << l << " " 
              << m << "\n";
   }

   cerr << "\ntesting inversion";
   for (long cnt = 0; cnt < 75; cnt++) {
      cerr << ".";
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(1000);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A, X;

      A.SetDims(n, n);

      FillRandom(A);


      vec_zz_p R;

      R.SetLength(n);
      for (long i = 0; i < n; i++) random(R[i]);

      zz_p d;

      inv(d, X, A);

      if (d != 0) {
	 if (R != A*(X*R)) 
	    cerr << "\n*\n*\n*\n*\n*********** oops " << len << " " << n << "\n";
      }
      else {
         cerr << "[singular]";
      }
   }

   cerr << "\ntesting solve";
   for (long cnt = 0; cnt < 75; cnt++) {
      cerr << ".";
 
      long len = RandomBnd(NTL_SP_NBITS-3)+4;
      long n = RandomBnd(1000);

      long p = RandomPrime_long(len);
      zz_p::init(p);

      Mat<zz_p> A;

      A.SetDims(n, n);
      FillRandom(A);

      Vec<zz_p> x, b;
      b.SetLength(n);
      FillRandom(b);

      zz_p d;

      solve(d, A, x, b);

      if (d != 0) {
	 if (A*x != b)
	    cerr << "\n*\n*\n*\n*\n*********** oops " << len << " " << n << "\n";
      }
      else {
         cerr << "[singular]";
      }
   }

   cerr << "\n";
}

