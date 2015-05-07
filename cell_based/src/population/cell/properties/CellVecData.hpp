/*

Copyright (c) 2005-2015, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef CELLVECDATA_HPP_
#define CELLVECDATA_HPP_

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <vector>

#include <petscvec.h>

#include "AbstractCellProperty.hpp"
#include "ChasteSerialization.hpp"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include "Exception.hpp"
#include "ArchiveLocationInfo.hpp"


class CellVecData;

/**
 * CellVecData class.
 *
 * This cell property allows each cell to store one or more 'named' PETSc Vec variables associated with it,
 * for example solutions to reaction-diffusion PDEs solved within the cell. Other classes may interrogate
 * or modify the values stored in this class.
 */
class CellVecData : public AbstractCellProperty
{
private:

    /**
     * The cell data.
     */
    std::map<std::string, Vec> mCellVecData;

    /**
     * Whether to free in the destructor the Vec stored in the map
     */
    bool mFreeVecs;

    /** Needed for serialization. */
    friend class boost::serialization::access;
    /**
     * Archive the member variables.
     *
     * @param archive the archive
     * @param version the current version of this class
     */
    template<class Archive>
    void serialize(Archive & archive, const unsigned int version)
    {
        archive & boost::serialization::base_object<AbstractCellProperty>(*this);
    }

public:

    /**
     * Default constructor
     */
    CellVecData();

    /**
     * Copy constructor used to perform a deep copy of mCellVecData
     */
    CellVecData(CellVecData& anotherCellVecData);

    /**
     * Constructor required for serialisation. Avoid using otherwise.
     */
    CellVecData(std::map<std::string, Vec>& cellVecData);

    /**
     * We need the empty virtual destructor in this class to ensure Boost
     * serialization works correctly with static libraries.
     */
    virtual ~CellVecData();

    /**
     * This assigns the cell data.
     *
     * @param variableName the name of the data to be set.
     * @param data the value to set it to.
     */
    void SetItem(const std::string& variableName, Vec data);

    /**
     * @return data.
     *
     * @param variableName the index of the data required.
     */
    Vec GetItem(const std::string& variableName) const;

    /**
     * @return number of data items
     */
    unsigned GetNumItems() const;

    /**
     * @return all keys.
     *
     * These are sorted in lexicographical/alphabetic order (so that the order is independent of any map iteration method).
     */
    std::vector<std::string> GetKeys() const;
};

#include "SerializationExportWrapper.hpp"
// Declare identifier for the serializer
CHASTE_CLASS_EXPORT(CellVecData)


namespace boost
{
namespace serialization
{
template<class Archive>
inline void save_construct_data(
    Archive & ar, const CellVecData * t, const unsigned int file_version)
{
    unsigned map_size = t->GetNumItems();
    ar << map_size;

    std::vector<std::string> keys = t->GetKeys();

    for (std::vector<std::string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        std::string key = *iter;
        ar << key;

        std::string archive_filename = ArchiveLocationInfo::GetArchiveDirectory() + key + ".vec";
        PetscTools::DumpPetscObject(t->GetItem(key), archive_filename);
    }

}

template<class Archive>
inline void load_construct_data(
    Archive & ar, CellVecData * t, const unsigned int file_version)
{
    unsigned map_size;
    ar >> map_size;

    std::map<std::string, Vec> archived_cell_vec_data;
    for (unsigned map_entry = 0; map_entry < map_size; ++map_entry)
    {
        std::string key;
        ar >> key;

        Vec archived_vec;
        std::string archive_filename = ArchiveLocationInfo::GetArchiveDirectory() + key + ".vec";
        PetscTools::ReadPetscObject(archived_vec, archive_filename);

        archived_cell_vec_data[key] = archived_vec;
    }

     ::new(t)CellVecData(archived_cell_vec_data);
}
}
}

#endif /* CELLVECDATA_HPP_ */
