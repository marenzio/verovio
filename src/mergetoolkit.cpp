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
    
    
    MergeToolkit::MergeToolkit( bool initFont ):Toolkit(initFont)
    {
        
    }
    
    bool MergeToolkit::MergeMeasures(Measure* m1, Measure* m2)
    {
            // may be needed for the future
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
                App* app = new App(EDITORIAL_NOTE);
                Lem* lem = new Lem();
                lem->AddLayer(layer_1);
                lem->SetSource("Italian");
                Rdg* rdg = new Rdg();
                rdg->AddLayer(layer_2);
                rdg->SetSource("English");
                app->AddLemOrRdg(lem);
                app->AddLemOrRdg(rdg);
                s1->AddEditorialElement(app);
                return true;
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
                if (dynamic_cast<Beam*>(note_1) != NULL && dynamic_cast<Beam*>(note_2) != NULL) {
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
            if (n1->m_children.size() > 0 && n1->m_children.size() > 0) {
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
        return true;
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
            lem->SetSource("Italian");
            Rdg* rdg = new Rdg();
            rdg->AddLayerElement(v2);
            rdg->SetSource("English");
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
        // Since we do no layout, we have only one page with one system
        Page* current_page = dynamic_cast<Page*>((&m_doc)->GetChild(pageNo));
        ArrayOfObjects systems = current_page->m_children;
        for (int i = 0; i < systems.size(); i++) {
            System* current_system = dynamic_cast<System*>(systems.at(i));
            ArrayOfObjects measures = current_system->m_children;
            for(int j = 0; j < measures.size(); j++) {
                // for now assume two voices/staffs
                Measure* current_measure = dynamic_cast<Measure*>(measures.at(j));
                ArrayOfObjects staffs = current_measure->m_children;
                Staff* staff_1 = dynamic_cast<Staff*>(staffs.at(0));
                Staff* staff_2 = dynamic_cast<Staff*>(staffs.at(1));
                // is_safe_merge checks whether it is safe to merge the lyrics of the whole measure
                // if it is, then the <app> will be added around the lyrics
                // otherwise, the <app> will be added around the whole layer and not the lyrics
                is_safe_merge = false;
                if (MergeStaffs(staff_1, staff_2)) {
                    is_safe_merge = true;
                    MergeStaffs(staff_1, staff_2);
                }
                is_safe_merge = false;
                staff_2->m_parent = NULL;
                current_measure->DetachChild(1);
            }
        }
        // need to remove 2nd staff from scoredef
        StaffGrp* staffgrp_layer1 = dynamic_cast<StaffGrp*>(m_doc.m_scoreDef.m_children.at(0));
        StaffGrp* staffgrp_layer2 = dynamic_cast<StaffGrp*>(staffgrp_layer1->m_children.at(0));
        staffgrp_layer2->RemoveChildAt(1);
        return true;
    }
 } //namespace vrv
