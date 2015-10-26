/////////////////////////////////////////////////////////////////////////////
// Name:        toolkit.cpp
// Author:      Laurent Pugin
// Created:     17/10/2013
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#include "mergetoolkit.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "editorial.h"
#include "iodarms.h"
#include "iomei.h"
#include "iopae.h"
#include "layer.h"
#include "page.h"
#include "measure.h"
#include "note.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "svgdevicecontext.h"
#include "style.h"
#include "system.h"
#include "verse.h"
#include "vrv.h"

namespace vrv {
    
    
    MergeToolkit::MergeToolkit( bool initFont):Toolkit(initFont)
    {
        source1 = "source1";
        source2 = "source2";
        
    }
    
    bool MergeToolkit::MergeMeasures(Measure* m1, Measure* m2)
    {
        if (m1== NULL && m2== NULL) {
        }
        else {
            ArrayOfObjects staffs1 = m1->m_children;
            ArrayOfObjects staffs2 = m2->m_children;
            if (staffs1.size() != staffs2.size()) {
                return false;
            }
            for (int k = 0; k < staffs1.size(); k++) {
                Staff* staff_1 = dynamic_cast<Staff*>(staffs1.at(k));
                Staff* staff_2 = dynamic_cast<Staff*>(staffs2.at(k));
                // is_safe_merge checks whether it is safe to merge the lyrics of the whole measure
                // if it is, then the <app> will be added around the lyrics
                // otherwise, the <app> will be added around the whole layer and not the lyrics
                is_safe_merge = false;
                if (MergeStaffs(staff_1, staff_2)) {
                    is_safe_merge = true;
                    MergeStaffs(staff_1, staff_2);
                }
                is_safe_merge = false;
            }
        }
        return true;
    }

    bool MergeToolkit::MergeStaffs(Staff* s1, Staff* s2)
    {
        // assume 1 layer
        Layer* layer_1 = dynamic_cast<Layer*>(s1->m_children.front());
        Layer* layer_2 = dynamic_cast<Layer*>(s2->m_children.front());
        if (layer_1 != NULL && layer_2 != NULL) {
            if (MergeLayers(layer_1, layer_2)) {
                return true;
            }
            else {
                // create an app for the two diff layers
                s1->DetachChild(0);
                s2->DetachChild(0);
                layer_1->m_parent = NULL;
                layer_2->m_parent = NULL;
                App* app = new App(EDITORIAL_STAFF);
                Lem* lem = new Lem();
                lem->AddLayer(layer_1);
                lem->SetSource(source1);
                Rdg* rdg = new Rdg();
                rdg->AddLayer(layer_2);
                rdg->SetSource(source2);
                app->AddLemOrRdg(lem);
                app->AddLemOrRdg(rdg);
                s1->AddEditorialElement(app);
                std::ofstream outfile;
                outfile.open("../diff.txt", std::ios::out | std::ios::app);
                outfile << StringFormat("Measure %d Staff %d is different.\n", (s1->m_parent->GetIdx() + 1- offset), (s1->GetStaffIdx() + 1));
                outfile.close();
                return false;
            }
        }
        return true;
    }
    
    bool MergeToolkit::MergeLayers(Layer* l1, Layer* l2)
    {
        ArrayOfObjects children_1 = l1->m_children;
        ArrayOfObjects children_2 = l2->m_children;
        int smaller = (int)(std::min(children_1.size(), children_2.size()));
        for (int k = 0; k < smaller; k++) {
            Note* note_1 = dynamic_cast<Note*>(children_1.at(k));
            Note* note_2 = dynamic_cast<Note*>(children_2.at(k));
            if (note_1 == NULL && note_2 == NULL)
            {
                Beam* beam_1 = dynamic_cast<Beam*>(children_1.at(k));
                Beam* beam_2 = dynamic_cast<Beam*>(children_2.at(k));

                if (beam_1 != NULL && beam_2 != NULL) {

                    if (!MergeBeams(beam_1, beam_2)) {
                        return false;
                    }
                }
                else {
                    Rest* rest_1 = dynamic_cast<Rest*>(children_1.at(k));
                    Rest* rest_2 = dynamic_cast<Rest*>(children_2.at(k));
                    if (rest_1 != NULL && rest_2 != NULL) {
                        if(!MergeRests(rest_1, rest_2)) {
                            return false;
                        }
                    }
                }
            }
            else if (note_1 == NULL || note_2 == NULL) {
                // note_1 and note_2 must both not be null, therefore this should not merge and return false
                return false;
            }
            else
            {
                if (!MergeNotes(note_1, note_2)) {
                    return false;
                    
                }
            }
        }
        return true;
        
    }
    
    bool MergeToolkit::MergeBeams(Beam* b1, Beam* b2)
    {
        int smaller = (int)(std::min(b1->m_children.size(), b2->m_children.size()));
        for (int k = 0; k < smaller; k++) {
            Note* note_1 = dynamic_cast<Note*>(b1->m_children.at(k));
            Note* note_2 = dynamic_cast<Note*>(b2->m_children.at(k));
            if (note_1 == NULL && note_2 == NULL)
            {
                Rest* rest_1 = dynamic_cast<Rest*>(b1->m_children.at(k));
                Rest* rest_2 = dynamic_cast<Rest*>(b2->m_children.at(k));
                if (rest_1 != NULL && rest_2 != NULL) {
                    if(!MergeRests(rest_1, rest_2)) {
                        return false;
                    }
                }
                
            }
            else if (note_1 == NULL || note_2 == NULL) {
                // note_1 and note_2 must both not be null, therefore this should not merge and return false
                return false;
            }
            else
            {
                if(!MergeNotes(note_1, note_2)) {
                    return false;
                }
            }
        }
        return true;
        
        
    }
    
    bool MergeToolkit::MergeNotes(Note* n1, Note* n2)
    {
        if (n1->GetActualDur() == n2->GetActualDur() &&
            n1->GetDiatonicPitch() == n2->GetDiatonicPitch()) {
            if (n1->m_children.size() > 0 && n2->m_children.size() > 0) {
                Verse* verse_1 = dynamic_cast<Verse*>(n1->m_children.front());
                Verse* verse_2 = dynamic_cast<Verse*>(n2->m_children.front());
                if (verse_1 != NULL && verse_2 != NULL) {
                    if (is_safe_merge) {
                        MergeVerses(verse_1, verse_2, n1, n2, 0);
                    }
                    else {
                        return true;
                    }
                }
                else {
                    verse_1 = dynamic_cast<Verse*>(n1->m_children.back());
                    verse_2 = dynamic_cast<Verse*>(n2->m_children.back());
                    if (verse_1 != NULL && verse_2 != NULL) {
                        if (is_safe_merge) {
                            MergeVerses(verse_1, verse_2, n1, n2, 1);
                        }
                        else {
                            return true;
                        }
                    }
                }
            }
        }
        else {
            return false;
        }
        return true;

        
    }

    bool MergeToolkit::MergeRests(Rest* r1, Rest* r2)
    {
        if (r1->GetDur() == r2->GetDur()) {
            return true;
        }
        else {
            return false;
        }
    }
    
    bool MergeToolkit::MergeVerses(Verse* v1, Verse* v2, Note* n1, Note* n2, int index)
    {
        if (v1 != NULL && v2!= NULL) {
            n1->DetachChild(index);
            n2->DetachChild(index);
            v1->m_parent = NULL;
            v2->m_parent = NULL;
            App* app = new App(EDITORIAL_NOTE);
            Lem* lem = new Lem();
            lem->AddLayerElement(v1);
            lem->SetSource(source1);
            Rdg* rdg = new Rdg();
            rdg->AddLayerElement(v2);
            rdg->SetSource(source2);
            app->AddLemOrRdg(lem);
            app->AddLemOrRdg(rdg);
            n1->AddEditorialElement(app);
        }
        else {
            return false;
        }
        return true;
    }
    
    
    
    bool MergeToolkit::Merge()
    {
        int pageNo = 0;
        offset = 0;
        // Since we do no layout, we have only one page with one system
        Page* current_page = dynamic_cast<Page*>((&m_doc)->GetChild(pageNo));
        Page* current_page2 = dynamic_cast<Page*>((&m_doc2)->GetChild(pageNo));
        ArrayOfObjects systems1 = current_page->m_children;
        ArrayOfObjects systems2 = current_page2->m_children;
        if (systems1.size() != systems2.size()) {
            return false;
        }
        for (int i = 0; i < systems1.size(); i++) {
            System* current_system1 = dynamic_cast<System*>(systems1.at(i));
            System* current_system2 = dynamic_cast<System*>(systems2.at(i));

            ArrayOfObjects measures1 = current_system1->m_children;
            ArrayOfObjects measures2 = current_system2->m_children;
            if (measures1.size() != measures2.size()) {
                return false;
            }
            for(int j = 0; j < measures1.size(); j++) {
                Measure* current_measure1 = dynamic_cast<Measure*>(measures1.at(j));
                Measure* current_measure2 = dynamic_cast<Measure*>(measures2.at(j));
                if (current_measure1 != NULL && current_measure2 != NULL) {
                    MergeMeasures(current_measure1, current_measure2);
                }
                else {
                    offset = offset + 1;
                }
            }
        }
        return true;
    }
    
    // identical to toolkit's load file, but must initialize another doc variable
    bool MergeToolkit::LoadOtherFile(const std::string &filename)
    {
        if ( IsUTF16( filename ) ) {
            return LoadOtherUTF16File( filename );
        }
        
        std::ifstream in( filename.c_str() );
        if (!in.is_open()) {
            return false;
        }
        
        in.seekg(0, std::ios::end);
        std::streamsize fileSize = (std::streamsize)in.tellg();
        in.clear();
        in.seekg(0, std::ios::beg);
        
        // read the file into the string:
        std::string content( fileSize, 0 );
        in.read(&content[0], fileSize);
        
        return LoadOtherString(content);
        
    }
    
    bool MergeToolkit::LoadOtherString(const std::string &content)
    {
        FileInputStream *input = NULL;
        input = new MeiInput( &m_doc2, "" );
        
        // something went wrong
        if ( !input ) {
            LogError( "Unknown error" );
            return false;
        }
        input->IgnoreLayoutInformation();
        
        // load the file
        if ( !input->ImportString( content )) {
            LogError( "Error importing data" );
            delete input;
            return false;
        }
        
        m_doc2.SetPageHeight( this->GetPageHeight() );
        m_doc2.SetPageWidth( this->GetPageWidth() );
        m_doc2.SetPageRightMar( this->GetBorder() );
        m_doc2.SetPageLeftMar( this->GetBorder() );
        m_doc2.SetPageTopMar( this->GetBorder() );
        m_doc2.SetSpacingStaff( this->GetSpacingStaff() );
        m_doc2.SetSpacingSystem( this->GetSpacingSystem() );
        
        m_doc2.PrepareDrawing();
        
        if (input->HasMeasureWithinEditoMarkup()) {
            LogWarning( "Only continous layout is possible with <measure> within editorial markup, switching to --no-layout" );
            this->SetNoLayout( true );
        }
        delete input;
        return true;
        
    }
    
    bool MergeToolkit::LoadOtherUTF16File( const std::string &filename )
    {
        /// Loading a UTF-16 file with basic conversion ot UTF-8
        /// This is called after checking if the file has a UTF-16 BOM
        
        LogWarning("The file seems to be UTF-16 - trying to convert to UTF-8");
        
        std::ifstream fin(filename.c_str(), std::ios::in | std::ios::binary);
        if (!fin.is_open()) {
            return false;
        }
        
        fin.seekg(0, std::ios::end);
        std::streamsize wfileSize = (std::streamsize)fin.tellg();
        fin.clear();
        fin.seekg(0, std::wios::beg);
        
        std::vector<unsigned short> utf16line;
        utf16line.reserve(wfileSize / 2 + 1);
        
        unsigned short buffer;
        while(fin.read((char *)&buffer, sizeof(unsigned short)))
        {
            utf16line.push_back(buffer);
        }
        //LogDebug("%d %d", wfileSize, utf8line.size());
        
        std::string utf8line;
        utf8::utf16to8(utf16line.begin(), utf16line.end(), back_inserter(utf8line));
        
        return LoadOtherString( utf8line );
    }

    
    bool MergeToolkit::SetSource1(std::string s1) {
        source1 = s1;
        return true;
    }
    
    bool MergeToolkit::SetSource2(std::string s2) {
        source2 = s2;
        return true;
    }
    
 } //namespace vrv
