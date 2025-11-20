#ifndef TEXT2D_H
#define TEXT2D_H
#include <string>
#include <world/object.h>
#include <memory>
#include <string>

class Text2D : public Object {
public:
    Text2D(std::string texturePath, std::shared_ptr<Shader> shaderProgram);
    void render(const std::string& text, int x, int y, int size);
};

#endif //TEXT2D_H
