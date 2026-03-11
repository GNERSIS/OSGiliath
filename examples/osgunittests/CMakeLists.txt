SET(TARGET_SRC 
    UnitTestFramework.cpp 
    UnitTests_osg.cpp 
    osgunittests.cpp 
    performance.cpp
    MultiThreadRead.cpp
    FileNameUtils.cpp
)

SET(TARGET_H 
    UnitTestFramework.hpp 
    performance.hpp
    MultiThreadRead.hpp
)

#### end var setup  ###

SETUP_COMMANDLINE_EXAMPLE(osgunittests)
