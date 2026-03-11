SET(TARGET_SRC MFC_OSG_MDI.cpp MFC_OSG_MDI.rc MFC_OSG_MDIDoc.cpp MFC_OSG_MDIView.cpp stdafx.cpp MFC_OSG.cpp MainFrm.cpp ChildFrm.cpp)
SET(TARGET_H   MFC_OSG_MDI.hpp   MFC_OSG_MDIDoc.hpp   MFC_OSG_MDIView.hpp   stdafx.hpp   MFC_OSG.hpp   MainFrm.hpp   ChildFrm.hpp Resource.hpp)

ADD_DEFINITIONS(-D_AFXDLL)
SET(REQUIRE_WINMAIN_FLAG 1)
          
#### end var setup  ###
SETUP_EXAMPLE(osgviewerMFC)
