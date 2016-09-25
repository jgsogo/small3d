/*
 *  SceneObject.hpp
 *
 *  Created on: 2014/10/19
 *      Author: Dimitri Kourkoulis
 *     License: BSD 3-Clause License (see LICENSE file)
 */

#pragma once

#include <vector>
#include "Model.hpp"
#include <memory>
#include "Logger.hpp"
#include "Image.hpp"
#include "BoundingBoxes.hpp"
#include <glm/glm.hpp>

using namespace std;

namespace small3d
{
  /**
   * @class	SceneObject
   *
   * @brief	An object that appears on the 3D scene. It is made up of one or more models (the latter for animation),
   * 			together with information about positioning and rotation and collision detection functionality.
   *
   */

  class SceneObject
  {
  private:
    vector<Model> model;
    bool animating;
    int frameDelay;
    int currentFrame;
    int framesWaited;
    int numFrames;
    Image texture;
    string name;

  public:

    /**
     * Constructor for object with texture
     * @param name The name of the object
     * @param modelPath The path to the file containing the object's model
     * @param texturePath The path to the file containing the object's texture. If the object
     * 					  is animated, it has to be the path up to the name part of the model.
     * 					  The program will append an underscore, a 6-digit index number and the
     * 					  .obj suffix for each frame. (e.g. goatAnim will become goatAnim_000001.obj,
     * 					  goatAnim_000002.obj, etc.)
     * @param numFrames The number of frames, if the object is animated. A single animation
     * 					sequence is supported per object and the first frame is considered to
     * 					be the non-moving state.
     * @param boundingBoxesPath The path to the file containing the object's bounding boxes. If no such
     * 							path is given, the object cannot be checked for collision detection.
     */
    SceneObject(string name, string modelPath, int numFrames = 1, string texturePath = "",
                string boundingBoxesPath = "");

    /**
     * Destructor
     */
    virtual ~SceneObject() = default;

    /**
     * Get the object's model
     * @return The object's model
     */
    Model& getModel() ;

    /**
     * Get the object's texture
     * @return The object's texture
     */
    const Image& getTexture() const;

    /**
     * Get the name of the object
     * @return The name of the object
     */
    const string getName();

    /**
     * Colour of the object (if not mapping a texture)
     */
    glm::vec4 colour;

    /**
     * Offset (position) of the object
     */
    glm::vec3 offset;

    /**
     * Rotation of the object (on x, y, z axes respectively)
     */
    glm::vec3 rotation;

    /**
     * Start animating the object
     */
    void startAnimating();

    /**
     * Stop animating the object
     */
    void stopAnimating();

    /**
     * Reset the animation sequence (go to first frame)
     */
    void resetAnimation();

    /**
     * Set the animation speed
     * @param delay The delay between each animation frame, expressed in number of game frames
     */
    void setFrameDelay(const int &delay);

    /**
     * Process animation (progress current frame if necessary)
     */
    void animate();

    /**
     * @brief	The bounding boxes for the object, used for collision detection.
     */

    BoundingBoxes boundingBoxes;

    /**
     * Check if the object collides with a point of the given
     * coordinates
     *
     * @param	x	The x coordinate of the point
     * @param	y	The y coordinate of the point
     * @param	z	The z coordinate of the point
     *
     * @return	true if a collision is detected, false otherwise.
     */

    bool collidesWithPoint(float x, float y, float z);

    /**
     * @fn	bool SceneObject::collidesWithSceneObject(SceneObject otherObject);
     *
     * @brief	Check if the object collides with another given object.
     *
     * @param	otherObject	The other object.
     *
     * @return	true if there is a collision, false if not.
     */

    bool collidesWithSceneObject(shared_ptr<SceneObject> otherObject);

  };

}
