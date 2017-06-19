//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InputLayoutCache.h: Defines InputLayoutCache, a class that builds and caches
// D3D11 input layouts.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
#define LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_

#include <GLES2/gl2.h>

#include <cstddef>

#include <array>
#include <map>

#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/SizedMRUCache.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"

namespace rx
{
struct PackedAttributeLayout;
}  // namespace rx

namespace std
{
template <>
struct hash<rx::PackedAttributeLayout>
{
    size_t operator()(const rx::PackedAttributeLayout &value) const
    {
        return angle::ComputeGenericHash(value);
    }
};
}  // namespace std

namespace gl
{
class Program;
}  // namespace gl

namespace rx
{
struct TranslatedAttribute;
struct TranslatedIndexData;
struct SourceIndexData;
class ProgramD3D;
class Renderer11;

struct PackedAttributeLayout
{
    PackedAttributeLayout();
    void addAttributeData(GLenum glType,
                          UINT semanticIndex,
                          gl::VertexFormatType vertexFormatType,
                          unsigned int divisor);

    bool operator==(const PackedAttributeLayout &other) const;

    enum Flags
    {
        FLAG_USES_INSTANCED_SPRITES     = 0x1,
        FLAG_INSTANCED_SPRITES_ACTIVE   = 0x2,
        FLAG_INSTANCED_RENDERING_ACTIVE = 0x4,
    };

    size_t numAttributes;
    unsigned int flags;
    std::array<uint32_t, gl::MAX_VERTEX_ATTRIBS> attributeData;
};

class InputLayoutCache : angle::NonCopyable
{
  public:
    InputLayoutCache();
    ~InputLayoutCache();

    void clear();

    gl::Error applyVertexBuffers(Renderer11 *renderer,
                                 const gl::State &state,
                                 const std::vector<TranslatedAttribute> &vertexArrayAttribs,
                                 const std::vector<TranslatedAttribute> &currentValueAttribs,
                                 GLenum mode,
                                 GLint start,
                                 TranslatedIndexData *indexInfo,
                                 GLsizei numIndicesPerInstance);

    gl::Error updateVertexOffsetsForPointSpritesEmulation(Renderer11 *renderer,
                                                          GLint startVertex,
                                                          GLsizei emulatedInstanceId);

    // Useful for testing
    void setCacheSize(size_t newCacheSize);

  private:
    gl::Error updateInputLayout(Renderer11 *renderer,
                                const gl::State &state,
                                GLenum mode,
                                const AttribIndexArray &sortedSemanticIndices,
                                GLsizei numIndicesPerInstance);
    gl::Error createInputLayout(Renderer11 *renderer,
                                const AttribIndexArray &sortedSemanticIndices,
                                GLenum mode,
                                gl::Program *program,
                                GLsizei numIndicesPerInstance,
                                d3d11::InputLayout *inputLayoutOut);

    // Starting cache size.
    static constexpr size_t kDefaultCacheSize = 1024;

    // The cache tries to clean up this many states at once.
    static constexpr size_t kGCLimit = 128;

    using LayoutCache = angle::base::HashingMRUCache<PackedAttributeLayout, d3d11::InputLayout>;
    LayoutCache mLayoutCache;

    std::vector<const TranslatedAttribute *> mCurrentAttributes;

    d3d11::Buffer mPointSpriteVertexBuffer;
    d3d11::Buffer mPointSpriteIndexBuffer;
};

}  // namespace rx

#endif // LIBANGLE_RENDERER_D3D_D3D11_INPUTLAYOUTCACHE_H_
