#if defined(_CLIENT_) || defined(_STANDALONE_)
#include "graphics/texture.hpp"
#include "resources/pixel-buffer.hpp"
#include <memory>
#include "logging.hpp"

namespace trillek {
namespace graphics {

Texture::~Texture() {
    Destroy();
}

Texture::Texture(std::weak_ptr<resource::PixelBuffer> pbp, bool dyn) : source_ptr(pbp) {
    texture_id = 0;
    gformat = 0;
    compare = false;
    dynamic = dyn;
    auto locked_ptr = pbp.lock();
    if(!locked_ptr) {
        LOGMSGC(WARNING) << "Failed to read image pointer";
    }
}

void Texture::Update() {
    std::shared_ptr<resource::PixelBuffer> locked_ptr = source_ptr.lock();
    if(locked_ptr) {
        if(!texture_id) {
            Load(*locked_ptr.get());
            locked_ptr->Validate();
        }
        else if(locked_ptr->IsDirty()) {
            Reload(locked_ptr->GetBlockBase(), locked_ptr->Width(), locked_ptr->Height());
            locked_ptr->Validate();
        }
    }
}

Texture::Texture(Texture && other) {
    texture_id = other.texture_id;
    compare = other.compare;
    gformat = other.gformat;
    dynamic = other.dynamic;
    source_ptr = std::move(other.source_ptr);
    other.texture_id = 0;
}

Texture& Texture::operator=(Texture && other) {
    texture_id = other.texture_id;
    compare = other.compare;
    gformat = other.gformat;
    dynamic = other.dynamic;
    source_ptr = std::move(other.source_ptr);
    other.texture_id = 0;
    return *this;
}

void Texture::Destroy() {
    if(texture_id) {
        glDeleteTextures(1, &texture_id);
    }
}

void Texture::Load(const resource::PixelBuffer & image) {
    CheckGLError();
    using resource::ImageColorMode;
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    switch(image.GetFormat()) {
    case ImageColorMode::COLOR_RGBA:
        gformat = GL_RGBA;
        break;
    case ImageColorMode::COLOR_RGB:
        gformat = GL_RGB;
        break;
    case ImageColorMode::MONOCHROME_A:
        gformat = GL_RG;
        break;
    case ImageColorMode::MONOCHROME:
        gformat = GL_RED;
        break;
    default:
        return;
    }
    const uint8_t * pixdata = image.GetBlockBase();
    if(nullptr == pixdata) {
        LOGMSGC(WARNING) << "Failed to read data pointer";
        return;
    }
    GLint magfilter = GL_LINEAR;
    for(auto &metaprop : image.meta) {
        if(metaprop.GetName() == "mag-filter") {
            if(metaprop.Is<std::string>()) {
                std::string filtermode = metaprop.Get<std::string>();
                if(filtermode == "nearest") {
                    magfilter = GL_NEAREST;
                }
            }
        }
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CheckGLError();
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, image.Width(), image.Height(), 0, gformat, GL_UNSIGNED_BYTE, pixdata);
    CheckGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void Texture::Reload(const uint8_t * image, GLuint width, GLuint height) {
    CheckGLError();
    if(!texture_id) {
        return;
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, width, height, 0, gformat, GL_UNSIGNED_BYTE, image);
}
void Texture::Generate(GLuint width, GLuint height, bool usealpha) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    CheckGLError();
    if(usealpha) {
        gformat = GL_RGBA;
    }
    else {
        gformat = GL_RGB;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, width, height, 0, gformat, GL_UNSIGNED_BYTE, nullptr);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateStencil(GLuint width, GLuint height) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CheckGLError();

    gformat = GL_STENCIL_INDEX;
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, width, height, 0, gformat, GL_UNSIGNED_BYTE, nullptr);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateDepth(GLuint width, GLuint height, bool stencil) {
    CheckGLError();
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    CheckGLError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    if(this->compare) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
    }
    CheckGLError();
    if(stencil) {
        gformat = GL_DEPTH_STENCIL;
    }
    else {
        gformat = GL_DEPTH_COMPONENT;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, gformat, width, height, 0, gformat, GL_UNSIGNED_BYTE, nullptr);
    CheckGLError();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::GenerateMultisample(GLuint width, GLuint height, GLuint samples) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    gformat = GL_RGBA;
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA, width, height, GL_FALSE);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture::GenerateMultisampleStencil(GLuint width, GLuint height, GLuint samples) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    gformat = GL_STENCIL_INDEX;
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_STENCIL_INDEX, width, height, GL_FALSE);
    CheckGLError();

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Texture::GenerateMultisampleDepth(GLuint width, GLuint height, GLuint samples, bool stencil) {
    if(!texture_id) {
        glGenTextures(1, &texture_id);
    }
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_id);
    CheckGLError();
    if(stencil) {
        gformat = GL_DEPTH_STENCIL;
    }
    else {
        gformat = GL_DEPTH_COMPONENT;
    }
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, gformat, width, height, GL_FALSE);
    CheckGLError();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

} // graphics
} // trillek
#endif // defined(_CLIENT_) || defined(_STANDALONE_)
