project(MedLogin)

mitk_create_executable(MedLogin DEPENDS 
                                        MitkCore 
                                        MitkAlgorithmsExt
                                        MitkQtWidgets
                                        MitkQtWidgetsExt
                                        MitkPlanarFigure
                                        MitkSegmentation
                                        )
