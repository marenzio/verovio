/////////////////////////////////////////////////////////////////////////////
// Name:        boundary.cpp
// Author:      Laurent Pugin
// Created:     2016
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "boundary.h"

//----------------------------------------------------------------------------

#include <assert.h>

//----------------------------------------------------------------------------

#include "functorparams.h"
#include "system.h"
#include "vrv.h"

namespace vrv {

//----------------------------------------------------------------------------
// BoundaryEnd
//----------------------------------------------------------------------------

BoundaryEnd::BoundaryEnd(Object *start)
{
    Reset();
    m_start = start;
    m_startClassName = start->GetClassName();
}

BoundaryEnd::~BoundaryEnd()
{
}

void BoundaryEnd::Reset()
{
    m_start = NULL;
    m_drawingMeasure = NULL;
}

std::string BoundaryEnd::GetClassName() const
{
    return m_startClassName + "BoundaryEnd";
}

//----------------------------------------------------------------------------
// BoundaryStartInterface
//----------------------------------------------------------------------------

BoundaryStartInterface::BoundaryStartInterface()
{
    Reset();
}

BoundaryStartInterface::~BoundaryStartInterface()
{
}

void BoundaryStartInterface::Reset()
{
    m_end = NULL;
    m_drawingMeasure = NULL;
}

void BoundaryStartInterface::SetEnd(BoundaryEnd *end)
{
    assert(!m_end);
    m_end = end;
}

//----------------------------------------------------------------------------
// BoundaryEnd functor methods
//----------------------------------------------------------------------------

int BoundaryEnd::PrepareBoundaries(FunctorParams *functorParams)
{
    PrepareBoundariesParams *params = dynamic_cast<PrepareBoundariesParams *>(functorParams);
    assert(params);

    // We need to its pointer to the last measure we have encountered
    if (params->m_lastMeasure == NULL) {
        LogWarning("A measure cannot be set to the end boundary");
    }
    this->SetMeasure(params->m_lastMeasure);

    // Endings are also set as Measure::m_drawingEnding for all meaasures in between - when we reach the end boundary of
    // an ending, we need to set the m_currentEnding to NULL
    if (params->m_currentEnding && this->GetStart()->Is() == ENDING) {
        params->m_currentEnding = NULL;
    }

    return FUNCTOR_CONTINUE;
}

int BoundaryEnd::ResetDrawing(FunctorParams *functorParams)
{
    this->SetMeasure(NULL);

    return FUNCTOR_CONTINUE;
};

int BoundaryEnd::CastOffSystems(FunctorParams *functorParams)
{
    CastOffSystemsParams *params = dynamic_cast<CastOffSystemsParams *>(functorParams);
    assert(params);

    // Since the functor returns FUNCTOR_SIBLINGS we should never go lower than the system children
    assert(dynamic_cast<System *>(this->m_parent));

    // Special case where we use the Relinquish method.
    // We want to move the measure to the currentSystem. However, we cannot use DetachChild
    // from the content System because this screws up the iterator. Relinquish gives up
    // the ownership of the Measure - the contentSystem will be deleted afterwards.
    BoundaryEnd *endBoundary = dynamic_cast<BoundaryEnd *>(params->m_contentSystem->Relinquish(this->GetIdx()));
    params->m_currentSystem->AddBoundaryEnd(endBoundary);

    return FUNCTOR_SIBLINGS;
}

//----------------------------------------------------------------------------
// Interface pseudo functor (redirected)
//----------------------------------------------------------------------------

int BoundaryStartInterface::InterfacePrepareBoundaries(FunctorParams *functorParams)
{
    PrepareBoundariesParams *params = dynamic_cast<PrepareBoundariesParams *>(functorParams);
    assert(params);

    // We have to be in a boundary start element
    assert(m_end);

    params->m_startBoundaries.push_back(this);

    return FUNCTOR_CONTINUE;
}

int BoundaryStartInterface::InterfaceResetDrawing(FunctorParams *functorParams)
{
    m_drawingMeasure = NULL;

    return FUNCTOR_CONTINUE;
};

} // namespace vrv