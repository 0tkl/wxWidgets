/////////////////////////////////////////////////////////////////////////////
// Name:        wx/generic/gridsel.h
// Purpose:     wxGridSelection
// Author:      Stefan Neis
// Created:     20/02/2000
// Copyright:   (c) Stefan Neis
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERIC_GRIDSEL_H_
#define _WX_GENERIC_GRIDSEL_H_

#include "wx/defs.h"

#if wxUSE_GRID

#include "wx/grid.h"

#include "wx/vector.h"

wxDEPRECATED_MSG("use wxGridBlockCoordsVector instead")
typedef wxVector<wxGridBlockCoords> wxVectorGridBlockCoords;

// Note: for all eventType arguments of the methods of this class wxEVT_NULL
//       may be passed to forbid events generation completely.
class WXDLLIMPEXP_CORE wxGridSelection
{
public:
    wxGridSelection(wxGrid *grid,
                    wxGrid::wxGridSelectionModes sel = wxGrid::wxGridSelectCells);

    bool IsSelection();
    bool IsInSelection(int row, int col) const;
    bool IsInSelection(const wxGridCellCoords& coords) const
    {
        return IsInSelection(coords.GetRow(), coords.GetCol());
    }

    void SetSelectionMode(wxGrid::wxGridSelectionModes selmode);
    wxGrid::wxGridSelectionModes GetSelectionMode() { return m_selectionMode; }
    void SelectRow(int row, const wxKeyboardState& kbd = wxKeyboardState());
    void SelectCol(int col, const wxKeyboardState& kbd = wxKeyboardState());
    void SelectBlock(int topRow, int leftCol,
                     int bottomRow, int rightCol,
                     const wxKeyboardState& kbd = wxKeyboardState(),
                     wxEventType eventType = wxEVT_GRID_RANGE_SELECTED);
    void SelectBlock(const wxGridCellCoords& topLeft,
                     const wxGridCellCoords& bottomRight,
                     const wxKeyboardState& kbd = wxKeyboardState(),
                     wxEventType eventType = wxEVT_GRID_RANGE_SELECTED)
    {
        SelectBlock(topLeft.GetRow(), topLeft.GetCol(),
                    bottomRight.GetRow(), bottomRight.GetCol(),
                    kbd, eventType);
    }

    // This function replaces all the existing selected blocks (which become
    // redundant) with a single block covering the entire grid.
    void SelectAll();

    void DeselectBlock(const wxGridBlockCoords& block,
                       const wxKeyboardState& kbd = wxKeyboardState(),
                       wxEventType eventType = wxEVT_GRID_RANGE_SELECTED);

    // Note that this method refreshes the previously selected blocks and sends
    // an event about the selection change.
    void ClearSelection();

    void UpdateRows( size_t pos, int numRows );
    void UpdateCols( size_t pos, int numCols );

    // Extend (or shrink) the current selection block (creating it if
    // necessary, i.e. if there is no selection at all currently or if the
    // current cell isn't selected, as in this case a new block
    // containing it is always added) to the one specified by the start and end
    // coordinates of its opposite corners (which don't have to be in
    // top/bottom left/right order).
    //
    // Note that blockStart is equal to wxGrid::m_currentCellCoords almost
    // always, but not always (the exception is when we scrolled out from
    // the top of the grid and select a column or scrolled right and select
    // a row: in this case the lowest visible row/column will be set as
    // current, not the first one).
    //
    // Both components of both blockStart and blockEnd must be valid.
    //
    // This function sends an event notifying about the selection change using
    // the provided event type, which is wxEVT_GRID_RANGE_SELECTED by default,
    // but may also be wxEVT_GRID_RANGE_SELECTING, when the selection is not
    // final yet.
    //
    // Return true if the current block was actually changed.
    bool ExtendCurrentBlock(const wxGridCellCoords& blockStart,
                            const wxGridCellCoords& blockEnd,
                            const wxKeyboardState& kbd,
                            wxEventType eventType = wxEVT_GRID_RANGE_SELECTED);


    // Return the coordinates of the cell from which the selection should
    // continue to be extended. This is normally the opposite corner of the
    // last selected block from the current cell coordinates.
    //
    // If there is no selection, just returns the current cell coordinates.
    wxGridCellCoords GetExtensionAnchor() const;

    wxGridCellCoordsArray GetCellSelection() const;
    wxGridCellCoordsArray GetBlockSelectionTopLeft() const;
    wxGridCellCoordsArray GetBlockSelectionBottomRight() const;
    wxArrayInt GetRowSelection() const;
    wxArrayInt GetColSelection() const;

    const wxGridBlockCoordsVector& GetBlocks() const { return m_selection; }

    void EndSelecting();
    void CancelSelecting();

    // A simple interface used by wxGrid::DrawSelection() as a helper to draw
    // the grid selection(s).
    class PolyPolygon
    {
    public:
        PolyPolygon() = default;
        ~PolyPolygon() = default;

        bool IsValid() const;

        // Return the number of polygons to draw.
        size_t GetSize() const { return m_counts.size(); }

        const int* GetCounts() const { return m_counts.data(); }

        const wxPoint* GetPoints() const { return m_points.data(); }

        void Append(const std::vector<wxPoint>& points);

        // Return the bounding box of the visible selected blocks. Return empty
        // rectangle if there is no selection.
        wxRect GetBoundingBox() const;

        void SetBoundingBox(const wxRect& rect);

    private:
        void CalcBoundingBox(const std::vector<wxPoint>& points);

        // m_counts contains the number of points in each of the polygons in m_points.

        std::vector<int>     m_counts;
        std::vector<wxPoint> m_points;

        int m_minX = 0, m_maxX = -1,
            m_minY = 0, m_maxY = -1;
    };

    // Return the PolyPolygon object. Call ComputePolyPolygon() if necessary.
    const PolyPolygon& GetPolyPolygon();

    void InvalidatePolyPolygon();

private:
    void SelectBlockNoEvent(const wxGridBlockCoords& block)
    {
        SelectBlock(block.GetTopRow(), block.GetLeftCol(),
                    block.GetBottomRow(), block.GetRightCol(),
                    wxKeyboardState(), wxEVT_NULL);
    }

    // Really select the block and don't check for the current selection mode.
    void Select(const wxGridBlockCoords& block,
                const wxKeyboardState& kbd,
                wxEventType eventType);

    // Ensure that the new "block" becomes part of "blocks", adding it to them
    // if necessary and, if we do it, also removing any existing elements of
    // "blocks" that become unnecessary because they're entirely contained in
    // the new "block". However note that we may also not to have to add it at
    // all, if it's already contained in one of the existing blocks.
    //
    // We don't currently check if the new block is contained by several
    // existing blocks, as this would be more difficult and doesn't seem to be
    // really needed in practice.
    void MergeOrAddBlock(wxGridBlockCoordsVector& blocks,
                         const wxGridBlockCoords& block);

    // Used by Select() and DeselectBlock() to ensure that we always have fewer
    // blocks selected in m_selection.
    static void MergeAdjacentBlocks(wxGridBlockCoordsVector& selection);

    // This function attempts to reduce the number of rectangles returned from
    // wxGrid::GetSelectedRectangles() before trying to convert them to PolyPolygon.
    // Most of the time this will result in just one rectangle.
    static void MergeAdjacentRects(std::vector<wxRect>& rectangles);

    // Called each time the selection changed or scrolled to recompute m_polyPolygon.
    void ComputePolyPolygon(bool refreshLabelWindows = false);

    // All currently selected blocks. We expect there to be a relatively small
    // amount of them, even for very large grids, as each block must be
    // selected by the user, so we store them unsorted.
    //
    // Selection may be empty, but if it isn't, the last block is special, as
    // it is the current block, which is affected by operations such as
    // extending the current selection from keyboard.
    wxGridBlockCoordsVector             m_selection;

    wxGrid                              *m_grid;
    wxGrid::wxGridSelectionModes        m_selectionMode;

    // Used by wxGrid::DrawSelection() to draw a:
    //
    // - Simple rectangle (using wxDC::DrawRectangle() if it is empty and the bounding box is valid.
    // - Simple polygon (using wxDC::DrawPolygon()) if it represents a simple polygon.
    // - Poly-polygon (using wxDC::DrawPolyPolygon()) if it consists of multiple polygons.
    //
    PolyPolygon                         m_polyPolygon;

    size_t                              m_isAnyLabelHighlighted = 0;

    wxDECLARE_NO_COPY_CLASS(wxGridSelection);
};

#endif  // wxUSE_GRID
#endif  // _WX_GENERIC_GRIDSEL_H_
