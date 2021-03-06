//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATABLETYPETRAITS_H
#define RAMSES_ANIMATABLETYPETRAITS_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Animation/AnimationCommon.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"
#include <cassert>

namespace ramses_internal
{
    template <typename EDataType>
    EDataType RoundFloat(Float val)
    {
        Float valOff = (val > 0.f ? val + 0.5f : val - 0.5f);
        return static_cast<EDataType>(valOff);
    }

    template <typename EDataType>
    class AnimatableTypeTraits
    {
    };

    template <>
    class AnimatableTypeTraits < bool >
    {
    public:
        static const bool IsIntegralType = true;
        static const UInt32 NumComponents = 1u;
        using DataType = bool;
        using ComponentType = bool;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return (val ? 1.f : 0.f);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return (val > 0.5f ? true : false);
        }
    };

    template <>
    class AnimatableTypeTraits < Int32 >
    {
    public:
        static const bool IsIntegralType = true;
        static const UInt32 NumComponents = 1u;
        using DataType = Int32;
        using ComponentType = Int32;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return static_cast<CorrespondingFloatType>(val);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return RoundFloat<DataType>(val);
        }
    };

    template <>
    class AnimatableTypeTraits < Int64 >
    {
    public:
        static const bool IsIntegralType = true;
        static const UInt32 NumComponents = 1u;
        using DataType = Int64;
        using ComponentType = Int64;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return static_cast<CorrespondingFloatType>(val);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return RoundFloat<DataType>(val);
        }
    };

    template <>
    class AnimatableTypeTraits < UInt32 >
    {
    public:
        static const bool IsIntegralType = true;
        static const UInt32 NumComponents = 1u;
        using DataType = UInt32;
        using ComponentType = UInt32;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return static_cast<CorrespondingFloatType>(val);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return RoundFloat<DataType>(val);
        }
    };

    template <>
    class AnimatableTypeTraits < UInt64 >
    {
    public:
        static const bool IsIntegralType = true;
        static const UInt32 NumComponents = 1u;
        using DataType = UInt64;
        using ComponentType = UInt64;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return static_cast<CorrespondingFloatType>(val);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return RoundFloat<DataType>(val);
        }
    };

    template <>
    class AnimatableTypeTraits < Float >
    {
    public:
        static const bool IsIntegralType = false;
        static const UInt32 NumComponents = 1u;
        using DataType = Float;
        using ComponentType = Float;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return val;
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return val;
        }
    };

    template <>
    class AnimatableTypeTraits < Double >
    {
    public:
        static const bool IsIntegralType = false;
        static const UInt32 NumComponents = 1u;
        using DataType = Double;
        using ComponentType = Double;
        using CorrespondingFloatType = Float;

        static ComponentType GetComponent(const DataType& val, EVectorComponent compIdx)
        {
            assert(compIdx == EVectorComponent_X);
            UNUSED(compIdx);
            return val;
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx == 0);
            UNUSED(compIdx);
            val = compVal;
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return static_cast<CorrespondingFloatType>(val);
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return static_cast<DataType>(val);
        }
    };

    template <>
    class AnimatableTypeTraits < Vector2 >
    {
    public:
        static const bool IsIntegralType = false;
        static const Int32 NumComponents = 2;
        using DataType = Vector2;
        using ComponentType = Float;
        using CorrespondingFloatType = Vector2;

        static ComponentType GetComponent(const Vector2& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            return (compIdx == EVectorComponent_X ? vec.x : vec.y);
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return val;
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return val;
        }
    };

    template <>
    class AnimatableTypeTraits < Vector3 >
    {
    public:
        static const bool IsIntegralType = false;
        static const Int32 NumComponents = 3;
        using DataType = Vector3;
        using ComponentType = Float;
        using CorrespondingFloatType = Vector3;

        static ComponentType GetComponent(const Vector3& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X: return vec.x;
            case EVectorComponent_Y: return vec.y;
            case EVectorComponent_Z: return vec.z;
            }
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            case EVectorComponent_Z:
                val.z = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return val;
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return val;
        }
    };

    template <>
    class AnimatableTypeTraits < Vector4 >
    {
    public:
        static const bool IsIntegralType = false;
        static const Int32 NumComponents = 4;
        using DataType = Vector4;
        using ComponentType = Float;
        using CorrespondingFloatType = Vector4;

        static ComponentType GetComponent(const Vector4& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X: return vec.x;
            case EVectorComponent_Y: return vec.y;
            case EVectorComponent_Z: return vec.z;
            case EVectorComponent_W: return vec.w;
            }
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            case EVectorComponent_Z:
                val.z = compVal;
                break;
            case EVectorComponent_W:
                val.w = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return val;
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return val;
        }
    };

    template <>
    class AnimatableTypeTraits < Vector2i >
    {
    public:
        static const bool IsIntegralType = true;
        static const Int32 NumComponents = 2;
        using DataType = Vector2i;
        using ComponentType = Int32;
        using CorrespondingFloatType = Vector2;

        static ComponentType GetComponent(const Vector2i& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            return (compIdx == 0u ? vec.x : vec.y);
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return CorrespondingFloatType(static_cast<Float>(val.x), static_cast<Float>(val.y));
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return DataType(RoundFloat<ComponentType>(val.x), RoundFloat<ComponentType>(val.y));
        }
    };

    template <>
    class AnimatableTypeTraits < Vector3i >
    {
    public:
        static const bool IsIntegralType = true;
        static const Int32 NumComponents = 3;
        using DataType = Vector3i;
        using ComponentType = Int32;
        using CorrespondingFloatType = Vector3;

        static ComponentType GetComponent(const Vector3i& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X: return vec.x;
            case EVectorComponent_Y: return vec.y;
            case EVectorComponent_Z: return vec.z;
            }
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            case EVectorComponent_Z:
                val.z = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return CorrespondingFloatType(static_cast<Float>(val.x), static_cast<Float>(val.y), static_cast<Float>(val.z));
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return DataType(RoundFloat<ComponentType>(val.x), RoundFloat<ComponentType>(val.y), RoundFloat<ComponentType>(val.z));
        }
    };

    template <>
    class AnimatableTypeTraits < Vector4i >
    {
    public:
        static const bool IsIntegralType = true;
        static const Int32 NumComponents = 4;
        using DataType = Vector4i;
        using ComponentType = Int32;
        using CorrespondingFloatType = Vector4;

        static ComponentType GetComponent(const Vector4i& vec, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X: return vec.x;
            case EVectorComponent_Y: return vec.y;
            case EVectorComponent_Z: return vec.z;
            case EVectorComponent_W: return vec.w;
            }
        }
        static void SetComponent(DataType& val, const ComponentType& compVal, EVectorComponent compIdx)
        {
            assert(compIdx < NumComponents);
            switch (compIdx)
            {
            default:
            case EVectorComponent_X:
                val.x = compVal;
                break;
            case EVectorComponent_Y:
                val.y = compVal;
                break;
            case EVectorComponent_Z:
                val.z = compVal;
                break;
            case EVectorComponent_W:
                val.w = compVal;
                break;
            }
        }
        static CorrespondingFloatType ToCorrespondingFloatType(DataType val)
        {
            return CorrespondingFloatType(static_cast<Float>(val.x), static_cast<Float>(val.y), static_cast<Float>(val.z), static_cast<Float>(val.w));
        }
        static DataType FromCorrespondingFloatType(CorrespondingFloatType val)
        {
            return DataType(RoundFloat<ComponentType>(val.x), RoundFloat<ComponentType>(val.y), RoundFloat<ComponentType>(val.z), RoundFloat<ComponentType>(val.w));
        }
    };
}

#endif
