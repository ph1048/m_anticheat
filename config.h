//#define SERVER_EVO
#define SERVER_TWINKLE
//#define SERVER_SPELLBORN
//#define SERVER_9EPISODE
//#define SERVER_ANTARES

#ifdef SERVER_TWINKLE

	#define DBGPRINT
	//#define CLICKERS //кликеры
	//#define CLICKERS_S //подавление защиты от кликеров
	//#define CRYPTO_S //подавление шифрования
	#define MAGNUM_FULL
	#define TW  //twinkle
	//#define ANTIDBG //антиотладка
	#define SERVER_IP "228.120.236.29" //tw 2 
	//#define SERVER_IP "127.0.0.1" //tw 2 
	#define MIN_DATAGRF_SIZE
	//#define EXE_MD5 "d572e02ec0d6375632300377a10eb420"

#endif

#ifdef SERVER_EVO

	#define ANTIDBG
	#define SERVER_IP "228.254.21.85"
	#define EXE_MD5 "6bef99a9c9fc59feb3ef516f6fc2b7dc"
	#define GRF_MD5 "1636fd366e3b009e02ea77d61d4d0fd5"
	#define K_XRAY

#endif

#ifdef SERVER_ANTARES

	#define CLICKERS
	#define ANTIDBG
	#define SERVER_IP "228.254.16.210"
	#define EXE_MD5 "6b2e501032f119ef7e05d310e71d2c86"
	#define K_XRAY
	#define MAGNUM_FULL
	#define ADD_GRF_CHECK || strcmp(CMD5Checksum::GetMD5((LPCSTR)"closed.grf"),"138a6a2ad61158e61ddf190c32fd730e")
	#define DATAINI
	#define GRF1 "closed.grf"
	#define GRF2 "custom.grf"
	#define GRF3 "anRO.grf"
	#define GRF4 "rdata.grf"
	#define GRF5 "sdata.grf"
	#define GRF6 "data.grf"
#endif

#ifdef SERVER_SPELLBORN

	//#define DBGPRINT
	#define ANTIDBG
	#define SERVER_IP "228.10.194.73"
	#define EXE_MD5 "a6f55023c9ba9444d4e52243d0bd922d" //sb
	//#define DATAINI

#endif

#ifdef SERVER_9EPISODE
	
	#define MAGNUM_FULL
	#define CLICKERS
	#define ANTIDBG
	#define SERVER_IP "228.223.133.5"
	#define EXE_MD5 "8ee97152d0c3b90da59cd664e1619586" //sb

#endif



