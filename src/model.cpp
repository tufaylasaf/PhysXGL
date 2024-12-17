#include "Model.h"

Model::Model(const char *file, std::string n, bool addToList)
{
    name = n;
    std::string text = get_file_contents(file);
    JSON = json::parse(text);

    Model::file = file;
    data = getData();

    traverseNode(0);

    LoadImGuiData("saveData/transforms.json");

    if (addToList)
        models.push_back(this);
}

Model::Model(const char *file, std::string tex, std::string n, bool addToList)
{
    name = n;
    texFolder = tex;
    std::string text = get_file_contents(file);
    JSON = json::parse(text);

    Model::file = file;
    data = getData();

    traverseNode(0);

    LoadImGuiData("saveData/transforms.json");

    if (addToList)
        models.push_back(this);
}

void Model::Draw(Shader &shader, Camera &camera)
{
    if (!display)
        return;
    bool textured = texFolder == "" ? false : true;
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Mesh::Draw(shader, camera, translation, rotation, scale, material, textured, matricesMeshes[i]);
    }
}

void Model::UI()
{
    if (ImGui::CollapsingHeader(name.c_str()))
    {
        ImGui::Checkbox("Visible", &display);

        // Position controls
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &translation[0], 0.1f);

        // Rotation controls
        glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation)); // Convert quaternion to Euler angles in degrees
        if (ImGui::DragFloat3("Rotation", &euler[0], 1.0f))
        {
            rotation = glm::quat(glm::radians(euler)); // Convert back to radians and quaternion
        }
        ImGui::DragFloat3("Scale", &scale[0], 0.1f);

        // Color controls
        ImGui::Text("Material");
        ImGui::ColorEdit3("Albedo", &material.albedo[0]);
        ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("AO", &material.ao, 0.0f, 1.0f);
    }
}

void Model::SaveImGuiData(const std::string &filename)
{
    std::ifstream file(filename);
    json saveData;

    // If the file exists, load it to update with new data
    if (file.is_open())
    {
        file >> saveData;
        file.close();
    }

    // Save the current transformation data
    saveData[name]["translation"] = {translation.x, translation.y, translation.z};

    glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));
    saveData[name]["rotation"] = {euler.x, euler.y, euler.z};

    saveData[name]["scale"] = {scale.x, scale.y, scale.z};
    saveData[name]["material"]["albedo"] = {material.albedo.x, material.albedo.y, material.albedo.z};
    saveData[name]["material"]["roughness"] = material.roughness;
    saveData[name]["material"]["metallic"] = material.metallic;
    saveData[name]["material"]["ao"] = material.ao;

    // Write the updated data to the file
    std::ofstream outFile(filename);
    if (outFile.is_open())
    {
        outFile << saveData.dump(4); // Save with 4 spaces indentation
        outFile.close();
    }
    else
    {
        std::cerr << "Unable to open file for saving transforms!" << std::endl;
    }
}

void Model::LoadImGuiData(const std::string &filename)
{
    std::ifstream file(filename);
    if (file.is_open())
    {
        json loadData;
        file >> loadData;
        file.close();

        // If the model name exists in the JSON file, load its transformations
        if (loadData.contains(name))
        {
            // Load translation
            if (loadData[name].contains("translation"))
            {
                translation = glm::vec3(
                    loadData[name]["translation"][0],
                    loadData[name]["translation"][1],
                    loadData[name]["translation"][2]);
            }

            // Load rotation (Euler angles -> quaternion)
            if (loadData[name].contains("rotation"))
            {
                glm::vec3 eulerAngles(
                    loadData[name]["rotation"][0],
                    loadData[name]["rotation"][1],
                    loadData[name]["rotation"][2]);
                rotation = glm::quat(glm::radians(eulerAngles)); // Convert degrees to radians and back to quaternion
            }

            // Load scale
            if (loadData[name].contains("scale"))
            {
                scale = glm::vec3(
                    loadData[name]["scale"][0],
                    loadData[name]["scale"][1],
                    loadData[name]["scale"][2]);
            }

            // Load material properties
            if (loadData[name].contains("material"))
            {
                if (loadData[name]["material"].contains("albedo"))
                {
                    material.albedo = glm::vec3(
                        loadData[name]["material"]["albedo"][0],
                        loadData[name]["material"]["albedo"][1],
                        loadData[name]["material"]["albedo"][2]);
                }

                if (loadData[name]["material"].contains("roughness"))
                {
                    material.roughness = loadData[name]["material"]["roughness"];
                }

                if (loadData[name]["material"].contains("metallic"))
                {
                    material.metallic = loadData[name]["material"]["metallic"];
                }

                if (loadData[name]["material"].contains("ao"))
                {
                    material.ao = loadData[name]["material"]["ao"];
                }
            }
        }
        else
        {
            std::cerr << "No saved transform or material data for model: " << name << std::endl;
        }
    }
    else
    {
        std::cerr << "Unable to open file for loading transforms and materials!" << std::endl;
    }
}

void Model::loadMesh(unsigned int indMesh)
{
    unsigned int posAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["POSITION"];
    unsigned int normalAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["NORMAL"];
    unsigned int texAccInd = JSON["meshes"][indMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
    unsigned int indAccInd = JSON["meshes"][indMesh]["primitives"][0]["indices"];

    std::vector<float> posVec = getFloats(JSON["accessors"][posAccInd]);
    std::vector<glm::vec3> positions = groupFloatsVec3(posVec);
    std::vector<float> normalVec = getFloats(JSON["accessors"][normalAccInd]);
    std::vector<glm::vec3> normals = groupFloatsVec3(normalVec);
    std::vector<float> texVec = getFloats(JSON["accessors"][texAccInd]);
    std::vector<glm::vec2> texUVs = groupFloatsVec2(texVec);

    std::vector<Vertex> vertices = assembleVertices(positions, normals, texUVs);
    std::vector<GLuint> indices = getIndices(JSON["accessors"][indAccInd]);
    std::vector<Texture> textures = getTextures();

    meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
    json node = JSON["nodes"][nextNode];

    glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
    if (node.find("translation") != node.end())
    {
        float transValues[3];
        for (unsigned int i = 0; i < node["translation"].size(); i++)
            transValues[i] = (node["translation"][i]);
        translation = glm::make_vec3(transValues);
    }
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (node.find("rotation") != node.end())
    {
        float rotValues[4] =
            {
                node["rotation"][3],
                node["rotation"][0],
                node["rotation"][1],
                node["rotation"][2]};
        rotation = glm::make_quat(rotValues);
    }
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
    if (node.find("scale") != node.end())
    {
        float scaleValues[3];
        for (unsigned int i = 0; i < node["scale"].size(); i++)
            scaleValues[i] = (node["scale"][i]);
        scale = glm::make_vec3(scaleValues);
    }
    glm::mat4 matNode = glm::mat4(1.0f);
    if (node.find("matrix") != node.end())
    {
        float matValues[16];
        for (unsigned int i = 0; i < node["matrix"].size(); i++)
            matValues[i] = (node["matrix"][i]);
        matNode = glm::make_mat4(matValues);
    }

    glm::mat4 trans = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 sca = glm::mat4(1.0f);

    trans = glm::translate(trans, translation);
    rot = glm::mat4_cast(rotation);
    sca = glm::scale(sca, scale);

    glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

    if (node.find("mesh") != node.end())
    {
        translationsMeshes.push_back(translation);
        rotationsMeshes.push_back(rotation);
        scalesMeshes.push_back(scale);
        matricesMeshes.push_back(matNextNode);

        loadMesh(node["mesh"]);
    }

    if (node.find("children") != node.end())
    {
        for (unsigned int i = 0; i < node["children"].size(); i++)
            traverseNode(node["children"][i], matNextNode);
    }
}

std::vector<unsigned char> Model::getData()
{
    std::string bytesText;
    std::string uri = JSON["buffers"][0]["uri"];

    std::string fileStr = std::string(file);
    std::string fileDirectory = fileStr.substr(0, fileStr.find_last_of('/') + 1);
    bytesText = get_file_contents((fileDirectory + uri).c_str());

    std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
    return data;
}

std::vector<float> Model::getFloats(json accessor)
{
    std::vector<float> floatVec;

    unsigned int buffViewInd = accessor.value("bufferView", 1);
    unsigned int count = accessor["count"];
    unsigned int accByteOffset = accessor.value("byteOffset", 0);
    std::string type = accessor["type"];

    json bufferView = JSON["bufferViews"][buffViewInd];
    unsigned int byteOffset = bufferView["byteOffset"];

    unsigned int numPerVert;
    if (type == "SCALAR")
        numPerVert = 1;
    else if (type == "VEC2")
        numPerVert = 2;
    else if (type == "VEC3")
        numPerVert = 3;
    else if (type == "VEC4")
        numPerVert = 4;
    else
        throw std::invalid_argument("Type is invalid (not SCALAR, VEC2, VEC3, or VEC4)");

    unsigned int beginningOfData = byteOffset + accByteOffset;
    unsigned int lengthOfData = count * 4 * numPerVert;
    for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i)
    {
        unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++]};
        float value;
        std::memcpy(&value, bytes, sizeof(float));
        floatVec.push_back(value);
    }

    return floatVec;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
    std::vector<GLuint> indices;

    unsigned int buffViewInd = accessor.value("bufferView", 0);
    unsigned int count = accessor["count"];
    unsigned int accByteOffset = accessor.value("byteOffset", 0);
    unsigned int componentType = accessor["componentType"];

    json bufferView = JSON["bufferViews"][buffViewInd];
    unsigned int byteOffset = bufferView["byteOffset"];

    unsigned int beginningOfData = byteOffset + accByteOffset;
    if (componentType == 5125)
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 4; i)
        {
            unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++]};
            unsigned int value;
            std::memcpy(&value, bytes, sizeof(unsigned int));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5123)
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i)
        {
            unsigned char bytes[] = {data[i++], data[i++]};
            unsigned short value;
            std::memcpy(&value, bytes, sizeof(unsigned short));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5122)
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accByteOffset + count * 2; i)
        {
            unsigned char bytes[] = {data[i++], data[i++]};
            short value;
            std::memcpy(&value, bytes, sizeof(short));
            indices.push_back((GLuint)value);
        }
    }

    return indices;
}

std::vector<Texture> Model::getTextures()
{
    std::vector<Texture> textures;
    if (texFolder != "")
    {

        std::string albedoPath = texFolder + "/albedo.png";
        std::string normalPath = texFolder + "/normal.png";
        std::string armPath = texFolder + "/arm.png";

        try
        {
            textures.push_back(Texture(albedoPath.c_str(), "albedo", 3));
            textures.push_back(Texture(normalPath.c_str(), "normal", 4));
            textures.push_back(Texture(armPath.c_str(), "arm", 5));
        }
        catch (const std::exception &e)
        {
            throw std::runtime_error(std::string("Failed to load texture: ") + e.what());
        }
    }

    return textures;
}

std::vector<Vertex> Model::assembleVertices(
    std::vector<glm::vec3> positions,
    std::vector<glm::vec3> normals,
    std::vector<glm::vec2> texUVs)
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < positions.size(); i++)
    {
        vertices.push_back(
            Vertex{
                positions[i],
                normals[i],
                texUVs[i]});
    }
    return vertices;
}

std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVec)
{
    std::vector<glm::vec2> vectors;
    for (int i = 0; i < floatVec.size(); i)
    {
        vectors.push_back(glm::vec2(floatVec[i++], floatVec[i++]));
    }
    return vectors;
}
std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVec)
{
    std::vector<glm::vec3> vectors;
    for (int i = 0; i < floatVec.size(); i)
    {
        vectors.push_back(glm::vec3(floatVec[i++], floatVec[i++], floatVec[i++]));
    }
    return vectors;
}
std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVec)
{
    std::vector<glm::vec4> vectors;
    for (int i = 0; i < floatVec.size(); i)
    {
        vectors.push_back(glm::vec4(floatVec[i++], floatVec[i++], floatVec[i++], floatVec[i++]));
    }
    return vectors;
}