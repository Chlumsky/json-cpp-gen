
#include "StringSwitchTree.h"

#include <vector>

typedef unsigned CharBitfield[(1<<(8*sizeof(char)))/(8*sizeof(unsigned))];
#define CHAR_BIT_ISSET(bitfield, c) (bitfield[(unsigned) (unsigned char) (c)/(8*sizeof(unsigned))]>>(unsigned(c)&(8*sizeof(unsigned)-1))&1u)
#define CHAR_BIT_SET(bitfield, c) (bitfield[(unsigned) (unsigned char) (c)/(8*sizeof(unsigned))] |= 1u<<(unsigned(c)&(8*sizeof(unsigned)-1)))

// labels must be unique!
std::unique_ptr<StringSwitchTree> StringSwitchTree::build(const std::string *labels, size_t count) {
    /*
     * This implementation heuristically switches the position with most unique values first.
     * It is however not perfectly optimal.
     * For example, for the following string set:
     *     aaa, bba, cca, dab, dbb, dcb, dac, dbc, dcc
     * Switching the second and then third character would be optimal (2 switches),
     * while this implementation would result in 7/3 == 2.333 switches on average.
     */

    if (!count)
        return nullptr;
    if (count == 1) {
        StringSwitchTree *leaf = new StringSwitchTree;
        leaf->position = LEAF_NODE_MARKER;
        leaf->label = *labels;
        return std::unique_ptr<StringSwitchTree>(leaf);
    }
    size_t minLength = labels->size(), maxLength = labels->size();
    for (const std::string *label = labels+1, *end = labels+count; label < end; ++label) {
        if (label->size() < minLength)
            minLength = label->size();
        if (label->size() > maxLength)
            maxLength = label->size();
    }

    // Find string position (or length) with most unique values and use it as root switch
    int commonPrefixLength = 0;
    int maxVariance = 0;
    int maxVariancePos = LEAF_NODE_MARKER;
    for (size_t i = 0; i < minLength; ++i) {
        CharBitfield charOccurences = { };
        int variance = 0;
        for (const std::string *label = labels, *end = labels+count; label < end; ++label) {
            if (!CHAR_BIT_ISSET(charOccurences, (*label)[i])) {
                ++variance;
                CHAR_BIT_SET(charOccurences, (*label)[i]);
            }
        }
        if (variance > maxVariance) {
            maxVariance = variance;
            maxVariancePos = int(i);
        }
        if (commonPrefixLength == i && variance == 1)
            ++commonPrefixLength;
    }
    if (maxVariance <= maxLength-minLength+1) {
        int lengthVariance = 0;
        std::vector<bool> lengthOccurences(maxLength+1, false);
        for (const std::string *label = labels, *end = labels+count; label < end; ++label) {
            if (!lengthOccurences[label->size()]) {
                ++lengthVariance;
                lengthOccurences[label->size()] = true;
            }
        }
        if (lengthVariance >= maxVariance) {
            maxVariance = lengthVariance;
            maxVariancePos = LENGTH_SWITCH_MARKER;
        }
    }

    // Build subtrees
    if (maxVariancePos >= 0) {
        StringSwitchTree *tree = new StringSwitchTree;
        tree->position = maxVariancePos;
        CharBitfield charOccurences = { };
        std::vector<std::string> subtreeLabels;
        subtreeLabels.reserve(count);
        for (const std::string *label = labels, *end = labels+count; label < end; ++label) {
            char c = (*label)[maxVariancePos];
            if (!CHAR_BIT_ISSET(charOccurences, c)) {
                subtreeLabels.clear();
                for (const std::string *subtreeLabel = label; subtreeLabel < end; ++subtreeLabel) {
                    if ((*subtreeLabel)[maxVariancePos] == c)
                        subtreeLabels.push_back(*subtreeLabel);
                }
                tree->branches[(int) (unsigned char) c] = build(subtreeLabels.data(), subtreeLabels.size());
                CHAR_BIT_SET(charOccurences, (*label)[maxVariancePos]);
            }
        }
        return std::unique_ptr<StringSwitchTree>(tree);
    }
    if (maxVariancePos == LENGTH_SWITCH_MARKER) {
        StringSwitchTree *tree = new StringSwitchTree;
        tree->position = LENGTH_SWITCH_MARKER;
        std::vector<bool> lengthOccurences(maxLength+1, false);
        std::vector<std::string> subtreeLabels;
        subtreeLabels.reserve(count);
        for (const std::string *label = labels, *end = labels+count; label < end; ++label) {
            size_t length = label->size();
            if (!lengthOccurences[length]) {
                subtreeLabels.clear();
                for (const std::string *subtreeLabel = label; subtreeLabel < end; ++subtreeLabel) {
                    if (subtreeLabel->size() == length)
                        subtreeLabels.push_back(*subtreeLabel);
                }
                tree->branches[int(length)] = build(subtreeLabels.data(), subtreeLabels.size());
                lengthOccurences[length] = true;
            }
        }
        return std::unique_ptr<StringSwitchTree>(tree);
    }

    return nullptr; // error (may happen if labels repeat)
}
