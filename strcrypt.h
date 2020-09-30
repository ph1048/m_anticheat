template <unsigned int n>
__forceinline const char* f(const char arr[n])
{
  static char y[n];
#define MakeNumber(a,b) a##b
#define Z(q,i) if(MakeNumber(q,i)<n)y[MakeNumber(q,i)]=arr[MakeNumber(q,i)]^0xAB;

#define QQ Z(q,0);Z(q,1);Z(q,2);Z(q,3);Z(q,4);Z(q,5);Z(q,6);Z(q,7);Z(q,8);Z(q,9);
#define q
  QQ
#undef q
#define q 1
  QQ
#undef q
#define q 2
  QQ
#undef q
#define q 3
  QQ
#undef q
#define q 4
  QQ
#undef q
#define q 5
  QQ
#undef q
#define q 6
  QQ
#undef q
#define q 7
  QQ
#undef q
#define q 8
  QQ
#undef q
#define q 9
  QQ

  for(volatile int j=0;j<n;j++)
	  y[j] ^= 0xAB;
  return y;
}