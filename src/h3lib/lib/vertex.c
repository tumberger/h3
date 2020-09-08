/*
 * Copyright 2020 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file  vertex.h
 *  @brief Functions for working with cell vertexes.
 */

#include "vertex.h"

#include "baseCells.h"
#include "faceijk.h"
#include "geoCoord.h"
#include "h3Index.h"

/** @brief Base cell vertex rotation table
 *
 * For each base cell, gives required CCW rotations to rotate
 * the vertexes on a given face to the orientation of the base
 * cell's home face. Note that while faces are not ordered for
 * non-pentagon cells, pentagons have their faces in directional
 * order, starting at J_AXES_DIGIT.
 */
static const BaseCellRotation
    baseCellVertexRotations[NUM_BASE_CELLS][MAX_BASE_CELL_FACES] = {
        {{0, 5}, {1, 0}, {2, 1}, {-1, 0}, {-1, 0}},     // base cell 0
        {{1, 5}, {2, 0}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 1
        {{0, 5}, {1, 0}, {2, 1}, {6, 3}, {-1, 0}},      // base cell 2
        {{1, 5}, {2, 0}, {3, 1}, {-1, 0}, {-1, 0}},     // base cell 3
        {{4, 5}, {0, 0}, {2, 3}, {1, 2}, {3, 4}},       // base cell 4 (pent)
        {{0, 5}, {1, 0}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 5
        {{1, 0}, {2, 1}, {6, 3}, {-1, 0}, {-1, 0}},     // base cell 6
        {{1, 5}, {2, 0}, {3, 1}, {7, 3}, {-1, 0}},      // base cell 7
        {{0, 0}, {1, 1}, {4, 5}, {-1, 0}, {-1, 0}},     // base cell 8
        {{1, 5}, {2, 0}, {7, 3}, {-1, 0}, {-1, 0}},     // base cell 9
        {{0, 5}, {1, 0}, {6, 3}, {-1, 0}, {-1, 0}},     // base cell 10
        {{1, 0}, {6, 3}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 11
        {{2, 5}, {3, 0}, {4, 1}, {-1, 0}, {-1, 0}},     // base cell 12
        {{2, 5}, {3, 0}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 13
        {{6, 3}, {11, 0}, {2, 1}, {7, 4}, {1, 0}},      // base cell 14 (pent)
        {{0, 1}, {3, 5}, {4, 0}, {-1, 0}, {-1, 0}},     // base cell 15
        {{0, 0}, {1, 1}, {4, 5}, {5, 3}, {-1, 0}},      // base cell 16
        {{1, 3}, {6, 0}, {11, 3}, {-1, 0}, {-1, 0}},    // base cell 17
        {{0, 0}, {1, 1}, {5, 3}, {-1, 0}, {-1, 0}},     // base cell 18
        {{2, 0}, {7, 3}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 19
        {{2, 3}, {7, 0}, {11, 3}, {-1, 0}, {-1, 0}},    // base cell 20
        {{2, 0}, {3, 1}, {7, 3}, {-1, 0}, {-1, 0}},     // base cell 21
        {{0, 0}, {4, 5}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 22
        {{1, 3}, {6, 0}, {10, 3}, {-1, 0}, {-1, 0}},    // base cell 23
        {{5, 3}, {10, 0}, {1, 1}, {6, 4}, {0, 0}},      // base cell 24 (pent)
        {{1, 3}, {6, 0}, {10, 3}, {11, 3}, {-1, 0}},    // base cell 25
        {{2, 5}, {3, 0}, {4, 1}, {8, 3}, {-1, 0}},      // base cell 26
        {{6, 3}, {7, 3}, {11, 0}, {-1, 0}, {-1, 0}},    // base cell 27
        {{3, 5}, {4, 0}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 28
        {{2, 5}, {3, 0}, {8, 3}, {-1, 0}, {-1, 0}},     // base cell 29
        {{0, 0}, {5, 3}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 30
        {{0, 1}, {3, 5}, {4, 0}, {9, 3}, {-1, 0}},      // base cell 31
        {{0, 3}, {5, 0}, {10, 3}, {-1, 0}, {-1, 0}},    // base cell 32
        {{0, 0}, {4, 5}, {5, 3}, {-1, 0}, {-1, 0}},     // base cell 33
        {{2, 3}, {7, 0}, {12, 3}, {-1, 0}, {-1, 0}},    // base cell 34
        {{6, 3}, {11, 0}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 35
        {{2, 3}, {7, 0}, {11, 3}, {12, 3}, {-1, 0}},    // base cell 36
        {{5, 3}, {6, 3}, {10, 0}, {-1, 0}, {-1, 0}},    // base cell 37
        {{7, 3}, {12, 0}, {3, 1}, {8, 4}, {2, 0}},      // base cell 38 (pent)
        {{6, 0}, {10, 3}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 39
        {{7, 0}, {11, 3}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 40
        {{0, 1}, {4, 0}, {9, 3}, {-1, 0}, {-1, 0}},     // base cell 41
        {{3, 0}, {4, 1}, {8, 3}, {-1, 0}, {-1, 0}},     // base cell 42
        {{3, 0}, {8, 3}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 43
        {{3, 5}, {4, 0}, {9, 3}, {-1, 0}, {-1, 0}},     // base cell 44
        {{6, 0}, {10, 3}, {11, 3}, {-1, 0}, {-1, 0}},   // base cell 45
        {{6, 3}, {7, 3}, {11, 0}, {16, 3}, {-1, 0}},    // base cell 46
        {{3, 3}, {8, 0}, {12, 3}, {-1, 0}, {-1, 0}},    // base cell 47
        {{0, 3}, {5, 0}, {14, 3}, {-1, 0}, {-1, 0}},    // base cell 48
        {{9, 3}, {14, 0}, {0, 1}, {5, 4}, {4, 0}},      // base cell 49 (pent)
        {{0, 3}, {5, 0}, {10, 3}, {14, 3}, {-1, 0}},    // base cell 50
        {{7, 3}, {8, 3}, {12, 0}, {-1, 0}, {-1, 0}},    // base cell 51
        {{5, 3}, {10, 0}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 52
        {{4, 0}, {9, 3}, {-1, 0}, {-1, 0}, {-1, 0}},    // base cell 53
        {{7, 3}, {12, 0}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 54
        {{7, 0}, {11, 3}, {12, 3}, {-1, 0}, {-1, 0}},   // base cell 55
        {{6, 3}, {11, 0}, {16, 3}, {-1, 0}, {-1, 0}},   // base cell 56
        {{5, 1}, {6, 3}, {10, 0}, {15, 3}, {-1, 0}},    // base cell 57
        {{8, 3}, {13, 0}, {4, 1}, {9, 4}, {3, 0}},      // base cell 58 (pent)
        {{6, 3}, {10, 0}, {15, 3}, {-1, 0}, {-1, 0}},   // base cell 59
        {{7, 3}, {11, 0}, {16, 3}, {-1, 0}, {-1, 0}},   // base cell 60
        {{4, 3}, {9, 0}, {14, 3}, {-1, 0}, {-1, 0}},    // base cell 61
        {{3, 3}, {8, 0}, {13, 3}, {-1, 0}, {-1, 0}},    // base cell 62
        {{11, 3}, {6, 0}, {15, 1}, {10, 4}, {16, 0}},   // base cell 63 (pent)
        {{3, 3}, {8, 0}, {12, 3}, {13, 3}, {-1, 0}},    // base cell 64
        {{4, 3}, {9, 0}, {13, 3}, {-1, 0}, {-1, 0}},    // base cell 65
        {{5, 3}, {9, 3}, {14, 0}, {-1, 0}, {-1, 0}},    // base cell 66
        {{5, 0}, {14, 3}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 67
        {{11, 3}, {16, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 68
        {{8, 0}, {12, 3}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 69
        {{5, 0}, {10, 3}, {14, 3}, {-1, 0}, {-1, 0}},   // base cell 70
        {{7, 3}, {8, 3}, {12, 0}, {17, 3}, {-1, 0}},    // base cell 71
        {{12, 3}, {7, 0}, {16, 1}, {11, 4}, {17, 0}},   // base cell 72 (pent)
        {{7, 3}, {12, 0}, {17, 3}, {-1, 0}, {-1, 0}},   // base cell 73
        {{5, 3}, {10, 0}, {15, 3}, {-1, 0}, {-1, 0}},   // base cell 74
        {{4, 3}, {9, 0}, {13, 3}, {14, 3}, {-1, 0}},    // base cell 75
        {{8, 3}, {9, 3}, {13, 0}, {-1, 0}, {-1, 0}},    // base cell 76
        {{11, 3}, {15, 1}, {16, 0}, {-1, 0}, {-1, 0}},  // base cell 77
        {{10, 3}, {15, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 78
        {{10, 3}, {15, 0}, {16, 5}, {-1, 0}, {-1, 0}},  // base cell 79
        {{11, 3}, {16, 0}, {17, 5}, {-1, 0}, {-1, 0}},  // base cell 80
        {{9, 3}, {14, 0}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 81
        {{8, 3}, {13, 0}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 82
        {{10, 3}, {5, 0}, {19, 1}, {14, 4}, {15, 0}},   // base cell 83 (pent)
        {{8, 0}, {12, 3}, {13, 3}, {-1, 0}, {-1, 0}},   // base cell 84
        {{5, 3}, {9, 3}, {14, 0}, {19, 3}, {-1, 0}},    // base cell 85
        {{9, 0}, {13, 3}, {-1, 0}, {-1, 0}, {-1, 0}},   // base cell 86
        {{5, 3}, {14, 0}, {19, 3}, {-1, 0}, {-1, 0}},   // base cell 87
        {{12, 3}, {16, 1}, {17, 0}, {-1, 0}, {-1, 0}},  // base cell 88
        {{8, 3}, {12, 0}, {17, 3}, {-1, 0}, {-1, 0}},   // base cell 89
        {{11, 3}, {15, 1}, {16, 0}, {17, 5}, {-1, 0}},  // base cell 90
        {{12, 3}, {17, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 91
        {{10, 3}, {15, 0}, {19, 1}, {-1, 0}, {-1, 0}},  // base cell 92
        {{15, 1}, {16, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 93
        {{9, 0}, {13, 3}, {14, 3}, {-1, 0}, {-1, 0}},   // base cell 94
        {{10, 3}, {15, 0}, {16, 5}, {19, 1}, {-1, 0}},  // base cell 95
        {{8, 3}, {9, 3}, {13, 0}, {18, 3}, {-1, 0}},    // base cell 96
        {{13, 3}, {8, 0}, {17, 1}, {12, 4}, {18, 0}},   // base cell 97 (pent)
        {{8, 3}, {13, 0}, {18, 3}, {-1, 0}, {-1, 0}},   // base cell 98
        {{16, 1}, {17, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 99
        {{14, 3}, {15, 5}, {19, 0}, {-1, 0}, {-1, 0}},  // base cell 100
        {{9, 3}, {14, 0}, {19, 3}, {-1, 0}, {-1, 0}},   // base cell 101
        {{14, 3}, {19, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 102
        {{12, 3}, {17, 0}, {18, 5}, {-1, 0}, {-1, 0}},  // base cell 103
        {{9, 3}, {13, 0}, {18, 3}, {-1, 0}, {-1, 0}},   // base cell 104
        {{12, 3}, {16, 1}, {17, 0}, {18, 5}, {-1, 0}},  // base cell 105
        {{15, 1}, {16, 0}, {17, 5}, {-1, 0}, {-1, 0}},  // base cell 106
        {{14, 3}, {9, 0}, {18, 1}, {13, 4}, {19, 0}},   // base cell 107 (pent)
        {{15, 0}, {19, 1}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 108
        {{15, 0}, {16, 5}, {19, 1}, {-1, 0}, {-1, 0}},  // base cell 109
        {{13, 3}, {18, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 110
        {{13, 3}, {17, 1}, {18, 0}, {-1, 0}, {-1, 0}},  // base cell 111
        {{14, 3}, {18, 1}, {19, 0}, {-1, 0}, {-1, 0}},  // base cell 112
        {{16, 1}, {17, 0}, {18, 5}, {-1, 0}, {-1, 0}},  // base cell 113
        {{14, 3}, {15, 5}, {18, 1}, {19, 0}, {-1, 0}},  // base cell 114
        {{13, 3}, {18, 0}, {19, 5}, {-1, 0}, {-1, 0}},  // base cell 115
        {{17, 1}, {18, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 116
        {{15, 5}, {19, 0}, {17, 3}, {18, 2}, {16, 4}},  // base cell 117 (pent)
        {{15, 5}, {18, 1}, {19, 0}, {-1, 0}, {-1, 0}},  // base cell 118
        {{13, 3}, {17, 1}, {18, 0}, {19, 5}, {-1, 0}},  // base cell 119
        {{18, 1}, {19, 0}, {-1, 0}, {-1, 0}, {-1, 0}},  // base cell 120
        {{17, 1}, {18, 0}, {19, 5}, {-1, 0}, {-1, 0}},  // base cell 121
};

/**
 * Get the number of CCW rotations of the cell's vertex numbers
 * compared to the directional layout of its neighbors.
 * @return Number of CCW rotations for the cell, or INVALID_ROTATIONS
 *         if the cell's base cell was not found on this face
 */
int vertexRotations(H3Index cell) {
    // Get the face and other info for the origin
    FaceIJK fijk;
    _h3ToFaceIjk(cell, &fijk);
    int baseCell = H3_EXPORT(h3GetBaseCell)(cell);
    int cellLeadingDigit = _h3LeadingNonZeroDigit(cell);

    for (int i = 0; i < MAX_BASE_CELL_FACES; i++) {
        BaseCellRotation rotation = baseCellVertexRotations[baseCell][i];
        if (rotation.face == fijk.face) {
            int ccwRot60 = rotation.ccwRot60;
            // Check whether the cell crosses a deleted pentagon subsequence
            if (_isBaseCellPentagon(baseCell)) {
                if (cellLeadingDigit == JK_AXES_DIGIT &&
                    fijk.face ==
                        baseCellVertexRotations[baseCell][IK_AXES_DIGIT - 2]
                            .face) {
                    // Crosses from JK to IK: Rotate CW
                    return ccwRot60 == 0 ? 5 : ccwRot60 - 1;
                }
                if (cellLeadingDigit == IK_AXES_DIGIT &&
                    fijk.face ==
                        baseCellVertexRotations[baseCell][JK_AXES_DIGIT - 2]
                            .face) {
                    // Crosses from IK to J: Rotate CCW
                    return (ccwRot60 + 1) % 6;
                }
            }
            return ccwRot60;
        }
    }
    // Failure case, should not be reachable
    return INVALID_ROTATIONS;  // LCOV_EXCL_LINE
}

/** @brief Hexagon direction to vertex number relationships (same face).
 *         Note that we don't use direction 0 (center).
 */
static const int directionToVertexNumHex[NUM_DIGITS] = {
    INVALID_DIGIT, 3, 1, 2, 5, 4, 0};

/** @brief Pentagon direction to vertex number relationships (same face).
 *         Note that we don't use directions 0 (center) or 1 (deleted K axis).
 */
static const int directionToVertexNumPent[NUM_DIGITS] = {
    INVALID_DIGIT, INVALID_DIGIT, 1, 2, 4, 3, 0};

/**
 * Get the first vertex number for a given direction. The neighbor in this
 * direction is located between this vertex number and the next number in
 * sequence.
 * @returns The number for the first topological vertex, or INVALID_VERTEX_NUM
 *          if the direction is not valid for this cell
 */
int vertexNumForDirection(const H3Index origin, const Direction direction) {
    int isPentagon = H3_EXPORT(h3IsPentagon)(origin);
    // Check for invalid directions
    if (direction == CENTER_DIGIT || direction >= INVALID_DIGIT ||
        (isPentagon && direction == K_AXES_DIGIT))
        return INVALID_VERTEX_NUM;
    // Determine the vertex number for the direction. If the origin and the base
    // cell are on the same face, we can use the constant relationships above;
    // if they are on different faces, we need to apply a rotation
    int rotations = vertexRotations(origin);
    // Handle bad rotation calculation: should be unreachable
    // LCOV_EXCL_START
    if (rotations == INVALID_ROTATIONS) return INVALID_VERTEX_NUM;
    // LCOV_EXCL_STOP
    // Find the appropriate vertex, rotating CCW if necessary
    return isPentagon ? (directionToVertexNumPent[direction] + NUM_PENT_VERTS -
                         rotations) %
                            NUM_PENT_VERTS
                      : (directionToVertexNumHex[direction] + NUM_HEX_VERTS -
                         rotations) %
                            NUM_HEX_VERTS;
}
