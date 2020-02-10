#include <iostream>
#include <string>
#include <omp.h>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <sstream>
#include <sys/time.h>
#include <iomanip>

class FileLoader	// Throws Error
{
	const char* filename ;

public:
	std::vector<char*> corpusdata ;

public:
	FileLoader( const char* file, std::string ( *cleaner ) ( const std::string& dirtystring ) ) ;
	void copycorpus( std::vector<std::string> &corpus ) ;
	std::string ( *cleaningtool ) ( const std::string &dirtystring ) ;
	static std::string makemytime( void );
	static inline double compute_elapsed( const struct timespec &starttime)
	{
		struct timespec endtime;
		clock_gettime( CLOCK_REALTIME, &endtime );
		double elapsed = (( endtime.tv_sec +
		                    endtime.tv_nsec / ( double ) 1000000000 ) -
		                  ( starttime.tv_sec +
		                    starttime.tv_nsec / ( double ) 1000000000 )) ;
		return elapsed;
	}
	static inline std::string makemytimebracketed(){ return( std::string("[") + makemytime() + "] " ) ;	}
	FILE *openblockreadfile( const std::string &filename );
	void closeblockreadfile( FILE *f );
	void loadcorpus( const char* filename );
	bool readblockreadfile( FILE *f, uint64_t preferredbufsize, std::string &prereaddata, std::string &buffer );
	void extractpreread( std::string &preread, std::string &buffer );
	void processblock( uint64_t &nread, uint64_t &nlines, const std::string &buffer );
	void computestarts( const std::string &buf, std::vector<uint64_t> &bufstarts ) ;
	void prepcorpus( const std::string &buf, std::vector<uint64_t> &bufstarts );
};
