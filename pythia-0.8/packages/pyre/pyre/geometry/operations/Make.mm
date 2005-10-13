# -*- Makefile -*-
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#
#                               Michael A.G. Aivazis
#                        California Institute of Technology
#                        (C) 1998-2005  All Rights Reserved
#
# <LicenseText>
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

PROJECT = pyre
PACKAGE = geometry/operations


#--------------------------------------------------------------------------
#

all: export

#--------------------------------------------------------------------------
# export

EXPORT_PYTHON_MODULES = \
    Binary.py \
    Composition.py \
    Difference.py \
    Dilation.py \
    Intersection.py \
    Reflection.py \
    Reversal.py \
    Rotation.py \
    Transformation.py \
    Translation.py \
    Union.py \
    __init__.py \


export:: export-package-python-modules

# version
# $Id: Make.mm,v 1.1.1.1 2005/03/08 16:13:46 aivazis Exp $

# End of file
