#include <stdlib.h>
#include <time.h>
#include <Shared/String.h>
#include <Shared/LoggerStream.h>
#include "Editor.h"
#include "NcursesModeOwner.h"

//
// main executable
//


    int
main1( int argc, char* argv [] )
    {  PSEUDOSTACK_START

    Logger::mInstance = new Logger ( Logger::WARN );
//     DEBUG_ONLY(    Logger::mInstance = LoggerStream::CreateFileStream( Logger::DEBUG, "ned.log", false );    )
    Logger::mInstance->mAutoFlush = true;

    // process arguments
    char* usage = "ARGUMENTS:  filepath?  line_number? ";
    if ( argc > 3 ) throw Exception ( usage, __FILE__, __FUNCTION__, __LINE__ );
    int a = 1;
    std::string filepath = "";
    int line_number = 0;
    if ( a < argc ) filepath = argv[a++];
    if ( a < argc )
        {
        line_number = atoi( argv[a++] );
        if ( line_number == 0 ) throw Exception ( usage, __FILE__, __FUNCTION__, __LINE__ );
        }

    NcursesModeOwner::NcursesMode( Settings::Get().key_mode_cbreak );

    // create and run editor
    Editor editor ( filepath, line_number );

    NcursesModeOwner::NormalMode();
    return 0;

    PSEUDOSTACK_END
    }




    int
main( int argc, char* argv [] )
    {
    #ifdef DEBUG
        return main1( argc, argv );
    #else
        try
            {
            return main1( argc, argv );
            }
        catch ( const std::exception& e )
            {
            std::cout << "\n\n" << "caught Exception:  " << e.what() << "\n\n";
            std::cout.flush();
            Logger::Log( e.what() , Logger::DEBUG, __FILE__, __FUNCTION__, __LINE__ );
            }
    #endif
    }


