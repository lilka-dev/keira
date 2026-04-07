#include "mjsresources.h"
#include <lilka.h>
#include "mjs.h"
#include "keira/ksound/sound.h"
#include "keira/ksound/audioplayer.h"

static String mjs_get_dir(struct mjs* mjs) {
    mjs_val_t dir_val = mjs_get(mjs, mjs_get_global(mjs), "__dir__", ~0);
    if (mjs_is_string(dir_val)) {
        size_t len;
        const char* s = mjs_get_string(mjs, &dir_val, &len);
        return String(s);
    }
    return "";
}

// resources.load_image(path[, transparencyColor[, pivotX, pivotY]])
// Returns {width, height, pointer}
static void mjs_resources_load_image(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);
    String fullPath = mjs_get_dir(mjs) + "/" + path;

    int32_t transparencyColor = -1;
    mjs_val_t tc_arg = mjs_arg(mjs, 1);
    if (mjs_is_number(tc_arg)) {
        transparencyColor = mjs_get_int(mjs, tc_arg);
    }

    int32_t pivotX = 0, pivotY = 0;
    mjs_val_t px_arg = mjs_arg(mjs, 2);
    mjs_val_t py_arg = mjs_arg(mjs, 3);
    if (mjs_is_number(px_arg) && mjs_is_number(py_arg)) {
        pivotX = mjs_get_int(mjs, px_arg);
        pivotY = mjs_get_int(mjs, py_arg);
    }

    lilka::Image* image = lilka::resources.loadImage(fullPath, transparencyColor, pivotX, pivotY);
    if (!image) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "width", ~0, mjs_mk_number(mjs, image->width));
    mjs_set(mjs, result, "height", ~0, mjs_mk_number(mjs, image->height));
    mjs_set(mjs, result, "pointer", ~0, mjs_mk_foreign(mjs, image));
    mjs_return(mjs, result);
}

// resources.rotate_image(image, angle, blankColor) -> {width, height, pointer}
static void mjs_resources_rotate_image(struct mjs* mjs) {
    mjs_val_t img = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, img, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Image* image = static_cast<lilka::Image*>(mjs_get_ptr(mjs, ptr_val));
    int16_t angle = mjs_get_int(mjs, mjs_arg(mjs, 1));
    int32_t blankColor = mjs_get_int(mjs, mjs_arg(mjs, 2));

    lilka::Image* rotatedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    image->rotate(angle, rotatedImage, blankColor);

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "width", ~0, mjs_mk_number(mjs, rotatedImage->width));
    mjs_set(mjs, result, "height", ~0, mjs_mk_number(mjs, rotatedImage->height));
    mjs_set(mjs, result, "pointer", ~0, mjs_mk_foreign(mjs, rotatedImage));
    mjs_return(mjs, result);
}

// resources.flip_image_x(image) -> {width, height, pointer}
static void mjs_resources_flip_image_x(struct mjs* mjs) {
    mjs_val_t img = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, img, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Image* image = static_cast<lilka::Image*>(mjs_get_ptr(mjs, ptr_val));

    lilka::Image* flippedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    image->flipX(flippedImage);

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "width", ~0, mjs_mk_number(mjs, flippedImage->width));
    mjs_set(mjs, result, "height", ~0, mjs_mk_number(mjs, flippedImage->height));
    mjs_set(mjs, result, "pointer", ~0, mjs_mk_foreign(mjs, flippedImage));
    mjs_return(mjs, result);
}

// resources.flip_image_y(image) -> {width, height, pointer}
static void mjs_resources_flip_image_y(struct mjs* mjs) {
    mjs_val_t img = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, img, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    lilka::Image* image = static_cast<lilka::Image*>(mjs_get_ptr(mjs, ptr_val));

    lilka::Image* flippedImage = new lilka::Image(image->width, image->height, image->transparentColor);
    image->flipY(flippedImage);

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "width", ~0, mjs_mk_number(mjs, flippedImage->width));
    mjs_set(mjs, result, "height", ~0, mjs_mk_number(mjs, flippedImage->height));
    mjs_set(mjs, result, "pointer", ~0, mjs_mk_foreign(mjs, flippedImage));
    mjs_return(mjs, result);
}

// resources.load_audio(path) -> {size, type, pointer}
static void mjs_resources_load_audio(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);
    String fullPath = mjs_get_dir(mjs) + "/" + path;

    String lowerPath = fullPath;
    lowerPath.toLowerCase();
    const char* type = NULL;
    if (lowerPath.endsWith(".mod")) {
        type = "mod";
    } else if (lowerPath.endsWith(".wav")) {
        type = "wav";
    } else if (lowerPath.endsWith(".mp3")) {
        type = "mp3";
    } else if (lowerPath.endsWith(".aac")) {
        type = "aac";
    } else if (lowerPath.endsWith(".flac")) {
        type = "flac";
    } else {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    FILE* file = fopen(fullPath.c_str(), "rb");
    if (!file) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize == 0) {
        fclose(file);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    uint8_t* data = new (std::nothrow) uint8_t[fileSize];
    if (!data) {
        fclose(file);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    size_t bytesRead = fread(data, 1, fileSize, file);
    fclose(file);

    if (bytesRead != fileSize) {
        delete[] data;
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    lilka::Sound* sound = new lilka::Sound(data, fileSize, type);

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "size", ~0, mjs_mk_number(mjs, fileSize));
    mjs_set(mjs, result, "type", ~0, mjs_mk_string(mjs, type, ~0, 1));
    mjs_set(mjs, result, "pointer", ~0, mjs_mk_foreign(mjs, sound));
    mjs_return(mjs, result);
}

// resources.delete(resource) - deletes image or sound
static void mjs_resources_delete(struct mjs* mjs) {
    mjs_val_t obj = mjs_arg(mjs, 0);
    mjs_val_t ptr_val = mjs_get(mjs, obj, "pointer", ~0);
    if (!mjs_is_foreign(ptr_val)) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    void* ptr = mjs_get_ptr(mjs, ptr_val);

    // Check if it has a "type" field (audio) or not (image)
    mjs_val_t type_val = mjs_get(mjs, obj, "type", ~0);
    if (mjs_is_string(type_val)) {
        // It's a sound
        lilka::Sound* sound = static_cast<lilka::Sound*>(ptr);
        if (lilka::audioPlayer.getPlayingSound() == sound) {
            lilka::audioPlayer.stop();
        }
        delete sound;
    } else {
        // It's an image
        delete static_cast<lilka::Image*>(ptr);
    }

    // Set pointer to null on the object
    mjs_set(mjs, obj, "pointer", ~0, mjs_mk_null());
    mjs_return(mjs, mjs_mk_undefined());
}

// resources.read_file(path) -> string
static void mjs_resources_read_file(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    const char* path = mjs_get_cstring(mjs, &arg0);
    String fullPath = mjs_get_dir(mjs) + "/" + path;

    String fileContent;
    int result = lilka::resources.readFile(fullPath, fileContent);
    if (result) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    mjs_return(mjs, mjs_mk_string(mjs, fileContent.c_str(), ~0, 1));
}

// resources.write_file(path, content)
static void mjs_resources_write_file(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    mjs_val_t arg1 = mjs_arg(mjs, 1);
    const char* path = mjs_get_cstring(mjs, &arg0);
    const char* content = mjs_get_cstring(mjs, &arg1);
    String fullPath = mjs_get_dir(mjs) + "/" + path;

    lilka::resources.writeFile(fullPath, content);
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_resources_register(struct mjs* mjs, const char* dir) {
    // Store dir as global __dir__
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "__dir__", ~0, mjs_mk_string(mjs, dir, ~0, 1));

    mjs_val_t resources = mjs_mk_object(mjs);
    mjs_set(mjs, resources, "load_image", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_load_image));
    mjs_set(mjs, resources, "rotate_image", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_rotate_image));
    mjs_set(mjs, resources, "flip_image_x", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_flip_image_x));
    mjs_set(mjs, resources, "flip_image_y", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_flip_image_y));
    mjs_set(mjs, resources, "load_audio", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_load_audio));
    mjs_set(mjs, resources, "delete", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_delete));
    mjs_set(mjs, resources, "read_file", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_read_file));
    mjs_set(mjs, resources, "write_file", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_resources_write_file));
    mjs_set(mjs, global, "resources", ~0, resources);
}
