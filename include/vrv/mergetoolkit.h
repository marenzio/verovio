/////////////////////////////////////////////////////////////////////////////
// Name:        toolkit.h
// Author:      Laurent Pugin
// Created:     17/10/2013
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef __VRV_MERGETOOLKIT_H__
#define __VRV_MERGETOOLKIT_H__

#include <string>

#ifdef USE_EMSCRIPTEN
#include "jsonxx.h"
#endif

#include "utf8.h"

//----------------------------------------------------------------------------

#include "doc.h"
#include "rest.h"
#include "toolkit.h"
#include "view.h"


namespace vrv {
    
    
    //----------------------------------------------------------------------------
    // Toolkit
    //----------------------------------------------------------------------------
    
    class MergeToolkit: public Toolkit
    {
    public:
        /**
         * @name Constructors and destructors
         */
        ///@{
        /** If initFont is set to false, Resources::InitFonts will have to be called explicitely */
        MergeToolkit( bool initFont = true );
        ///@}
      
        
        /**
         * Merge the two voices into one
         * Return true
         */
        bool Merge();
        
        bool LoadOtherFile(const std::string &filename);
                
        bool SetSource1(std::string s1);
        
        bool SetSource2(std::string s2);
        
        
    private:
        /**
         * Changed methods from void to bool
         * because it may be useful later on
         * when merging the notes that are not exactly the same
         */
        bool MergeMeasures(Measure* m1, Measure* m2);
        
        bool MergeStaffs(Staff* s1, Staff* s2);
        
        bool MergeLayers(Layer* l1, Layer* l2);

        bool MergeBeams(Beam* b1, Beam* b2);
        
        bool MergeNotes(Note* n1, Note* n2);
        
        bool MergeRests(Rest* r1, Rest* r2);
        
        bool MergeVerses(Verse* v1, Verse* v2, Note* n1, Note* n2, int index);

        
    private:
        bool is_safe_merge;
        std::string source1;
        std::string source2;
        
        int offset; // this is to figure out how to offset the measure numbers when a scoredef is added
        
    protected:
        Doc m_doc2;
        
        
    };
    
}// namespace vrv
#endif
