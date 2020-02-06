#include "fastFileLoader.h"

using namespace std ;

FileLoader::FileLoader( const char* file, string ( *cleaner ) ( const string& dirtystring ) ) : filename( file ),
                                                                                                cleaningtool( *cleaner )
  {
  loadcorpus( filename ) ;
  }

FileLoader::FileLoader( const char* file, string ( *cleaner ) ( const string& dirtystring ), vector<char*> &corpus ) : filename( file ),
                                                                                                                       cleaningtool( *cleaner )
  {
  loadcorpus( filename ) ;
  corpus.swap( corpusdata ) ;
  }

std::string FileLoader::makemytime( void )
  {
  struct timeval tv ;
  struct tm brokendowntime ;
  std::stringstream ss ;
  long partialsec ;

  gettimeofday( &tv, NULL ) ;
  gmtime_r( &tv.tv_sec, &brokendowntime ) ;
  partialsec = 1000 * tv.tv_usec * 0.000001 ;

  ss << std::setfill( '0' ) << std::setw( 4 ) << brokendowntime.tm_year + 1900 << "-" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_mon + 1     << "-" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_mday        << "T" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_hour        << ":" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_min         << ":" <<
        std::setfill( '0' ) << std::setw( 2 ) << brokendowntime.tm_sec         << "." <<
        std::setfill( '0' ) << std::setw( 3 ) << partialsec                    << "Z" ;

  return( ss.str() ) ;
  }

FILE *FileLoader::openblockreadfile( const string &filename )
  {
  static const string catter( "/usr/bin/ucat" ) ;
  FILE *f ;
  string command ;

  command = catter + " " + filename + " 2>/dev/null < /dev/null" ;
  f = popen( command.c_str(), "r" ) ;

  return( f ) ;
  }

void FileLoader::closeblockreadfile( FILE *f )
  {
  if( f != NULL )
    pclose( f ) ;
  }

void FileLoader::loadcorpus( const char* filename )
  {
  struct timespec loadstarttime ;
  struct timespec stepstarttime ;
  cout << makemytimebracketed() << "Loading Corpus \"" << filename << "\"..." << endl ;
  clock_gettime( CLOCK_REALTIME, &loadstarttime ) ;
  clock_gettime( CLOCK_REALTIME, &stepstarttime ) ;
  corpusdata.reserve( 62000000 ) ;

  FILE *f = openblockreadfile( filename ) ;
  
  if( f != NULL )
    {
    const uint64_t preferredbufsize = 32UL * 1024ULL * 1024ULL ;
    const uint64_t reportinterval = 10000000ULL ;
    uint64_t nextreport = reportinterval ;
    string buffer[2] ;
    uint64_t switchbuffer = 0 ;
    bool bufferread ;
    string preread ;
    uint64_t nlinesinfile = 0 ;
    uint64_t nbytesinfile = 0 ;
    
    bufferread = readblockreadfile( f, preferredbufsize, preread, buffer[ switchbuffer ] ) ;
#ifdef _OPENMP
    omp_set_nested( 1 ) ; // Need this for nested parallelism
#endif
#pragma omp parallel num_threads( 2 )
    {
    while( bufferread )
      {
#pragma omp single nowait
      bufferread = readblockreadfile( f, preferredbufsize, preread, buffer[ !switchbuffer ] ) ;

#pragma omp single
      {
      processblock( nbytesinfile, nlinesinfile, buffer[ switchbuffer ] ) ;
      if( nlinesinfile >= nextreport )
        {
        cout << makemytimebracketed() << "  ...processed " << nlinesinfile << " corpus data" << endl ;

        nextreport = nlinesinfile + reportinterval ;
        }
      switchbuffer = !switchbuffer ;
      } // end of single, relying on implied barrier
      } // end of while 
    } // end of parallel
#ifdef _OPENMP
    omp_set_nested( 0 ) ; // switching off nested parallelism
#endif
    processblock( nbytesinfile, nlinesinfile, buffer[ switchbuffer ] ) ;

    closeblockreadfile( f ) ;

    cout << makemytimebracketed() << "...[1/4] Complete (" << compute_elapsed( stepstarttime ) << " seconds)" << "\n" ;
    cout << makemytimebracketed() << "         Filesize:             " << nbytesinfile << "\n" ;
    cout << makemytimebracketed() << "         Lines in File      " << nlinesinfile << "\n" ;
    cout << flush ;
    cout << makemytimebracketed() << "...Complete (" << compute_elapsed( stepstarttime ) << " seconds)" << endl ;
    }
  else
    throw "Error: Could not load corpus\n" ;
  }

bool FileLoader::readblockreadfile( FILE *f, uint64_t preferredbufsize, string &prereaddata, string &buffer )
  {
  const uint64_t minbufsize = 1024UL * 1024UL ;
  uint64_t npreread = prereaddata.size() ;
  uint64_t nreserve ;
  bool filehasmoredata = true ;
  buffer.clear() ;

  nreserve = npreread + minbufsize ;
  if( nreserve < preferredbufsize )
    nreserve = preferredbufsize ;

  if( f == NULL )               // better not happen
    {
    buffer = prereaddata ;
    filehasmoredata = false ;
    }
  else
    {
    buffer.resize( nreserve ) ;
    for( uint64_t i = 0 ; i < npreread ; ++i )
      buffer[ i ] = prereaddata[ i ] ;

    uint64_t maxread = nreserve - npreread ;
    uint64_t nread = fread( ( void * ) &( buffer[ npreread ] ), 1, maxread, f ) ;
    if( nread < maxread )
      {
      buffer.resize( npreread + nread ) ;
      if( ( buffer.size() > 0 ) && ( buffer[ buffer.size() - 1 ] != '\n' ) )
        buffer.push_back( '\n' ) ;
      filehasmoredata = false ;
      }
    else
      extractpreread( prereaddata, buffer ) ;
    }

  return( filehasmoredata ) ;
  }

void FileLoader::extractpreread( string &preread, string &buffer )
  {
  preread.clear() ;
  uint64_t buffersize = buffer.size() ;

  if( ( buffersize > 0 ) && ( buffer[ buffersize - 1 ] != '\n' ) )
    {
    // Find last newline in buffer
    uint64_t prereadsize = 0 ;
    for( uint64_t i = 0 ; i < buffersize ; ++i )
      {
      if( buffer[ buffersize - i - 1 ] == '\n' )
        break ;
      ++prereadsize ;
      }

    if( prereadsize < buffersize )
      {                 // Good.  preread will contain the last (partial) line
      preread.clear() ;
      preread.reserve( prereadsize ) ;

      for( uint64_t i = buffersize - prereadsize ; i < buffersize ; ++i )
        preread.push_back( buffer[ i ] ) ;
      }
    else  // No newline in buffer, it's all preread and buffer does contain previous preread data
      preread = buffer ;

    buffer.resize( buffersize - prereadsize ) ;
    }
  }

void FileLoader::processblock( uint64_t &nread, uint64_t &nlines, const string &buffer )
  {
  uint64_t nbuffer ;
  uint64_t nlinesinbuffer = 0 ;

  nbuffer = buffer.size() ;

#pragma omp parallel
  {
  uint64_t  mycount = 0 ;
#pragma omp for schedule( static ) nowait
  for( uint64_t i = 0 ; i < nbuffer ; ++i )
    if( buffer[ i ] == '\n' )
      ++mycount ;

#pragma omp atomic
  nlinesinbuffer += mycount ;
  } // parallel

  if( nbuffer > 0 )
    {
    // Must have parallel here rather than in the routines as
    // thread count in parallel regions must not change during
    // this processing

    vector<uint64_t> bufstarts ;

#pragma omp parallel
      {
#pragma omp single
      computestarts( buffer, bufstarts ) ;
      
      prepcorpus( buffer, bufstarts ) ;
      } // End of Parallel, implied barrier
    }

  nread += nbuffer ;
  nlines += nlinesinbuffer ;
  }

void FileLoader::computestarts( const string &buf, vector<uint64_t> &bufstarts )
  {
#ifdef _OPENMP
  uint64_t nthread = omp_get_num_threads() ;
#else
  uint64_t nthread = 1 ;
#endif
  uint64_t len     = buf.size() ;

  uint64_t nper = buf.size() / nthread ;
  if( nper == 0 )
    nper = 1 ;

  bufstarts.resize( nthread + 1 ) ;
  bufstarts[ nthread ] = len ;
  bufstarts[ 0 ] = 0 ;
  for( uint64_t thread = 1 ; thread < nthread ; ++thread )
    {
    uint64_t offset = bufstarts[ thread - 1 ] + nper ;
    for( ; offset < len ; ++offset )
      if( buf[ offset ] == '\n' )
        break ;

    bufstarts[ thread ] = ( offset < len ) ? offset + 1 : len ;
    }
  }

void FileLoader::prepcorpus( const string &buf, vector<uint64_t> &bufstarts )  // Inside parallel region
  {
  vector<char*> mycorpusentries ;
  mycorpusentries.reserve( 2000000 ) ;
  string entrystr ;
#ifdef _OPENMP
  uint64_t myid = omp_get_thread_num() ;
#else
  uint64_t myid = 1 ;  
#endif  
  uint64_t stop = bufstarts[ myid + 1 ] ;

  entrystr.reserve( 512 ) ;

  for( uint64_t i = bufstarts[ myid ] ; i < stop ; )
    {
    uint64_t j = i ;
    while( ( i < stop ) && ( buf[ i ] != '\n' ) )
      ++i ;

    if( i > j )
      {
      entrystr.resize( i - j ) ;
      for( uint64_t k = j ; k < i ; ++k )
        entrystr[ k - j ] = buf[ k ] ;

      entrystr = cleaningtool( entrystr ) ;
      char* temp ;
      temp = new char[ entrystr.length() + 1 ] ;
      strcpy( temp, entrystr.c_str() ) ;
      mycorpusentries.push_back( temp ) ;
      }
    if( i < stop )
      ++i ;
    }

#pragma omp critical( globalize_corpus )
    {
    uint32_t corpussize = corpusdata.size() ;
    uint32_t mycorpusentriessize = mycorpusentries.size() ;
    corpusdata.resize( corpussize + mycorpusentriessize ) ;
    for( uint32_t i = 0 ; i < mycorpusentriessize ; ++i )
      corpusdata[ corpussize + i ] = mycorpusentries[ i ] ;
    } // end of critical

    {
    vector<char*>().swap( mycorpusentries ) ;
    }
#pragma omp barrier
  }
