SET(TARGET_SRC 
  ShapeToGeometry.cpp
  AggregateGeometryVisitor.cpp
  osggpucull.cpp
)

SET(TARGET_H
  ShapeToGeometry.hpp
  AggregateGeometryVisitor.hpp
  GpuCullShaders.hpp
)

#### end var setup  ###
SETUP_EXAMPLE(osggpucull)
