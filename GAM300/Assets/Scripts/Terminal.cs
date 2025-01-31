﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BeanFactory;
using GlmSharp;

public class Terminal : Script
{
    public int index;
    public vec3 colors;
    public GameObject glowObjectReference;
    public MeshRenderer terminalglowMesh;
    public MeshRenderer glowMesh;
    public Material glowMat;
    public bool interaced = false;
    public bool interactOnce = true;

    void Start()
    {
        //get the glow color
        glowMat = glowObjectReference.GetComponent<MeshRenderer>().material;
        //glowMat = glowMesh.material;

        if (terminalglowMesh == null)
        {
            Console.WriteLine("Missing GlowMesh Reference");
        }
        if (glowMat == null)
        {
            Console.WriteLine("Missing GlowMat Reference");
        }
    }

    void Update()
    {
        CheckCheckpoint();
    }

    void OnTriggerEnter(PhysicsComponent rb)
    {
        //detect the player
        if (GetTag(rb) == "Player")
        {
            Console.WriteLine("AtCheckpoint");
            ThirdPersonController.instance.checkpointIndex = index;
            ThirdPersonController.instance.spawnPoint = transform.localPosition + transform.forward * 2f + vec3.UnitY * 0.5f;
            //CheckCheckpoint();

            ThirdPersonController.instance.isAtCheckpoint = true;
        }

    }

    public void CheckCheckpoint()
    {
        if(index == ThirdPersonController.instance.checkpointIndex)
        {
            //save checkpoint
            if (interactOnce && ThirdPersonController.instance.isAtCheckpoint == true)
            {
                interactOnce = false;
                Console.WriteLine("Save Checkpoint");
                //change glow of terminal
                Material mat = terminalglowMesh.material;
                mat.Set(glowMat);
                mat.color = new vec4(glowMat.color);

                //mat.metallic = metallic;

                //shift the spawn point to where the current termainal position where the player save
                AudioManager.instance.obj_success.Play();

                ThirdPersonController.instance.currentHealth = ThirdPersonController.instance.maxHealth;
                ThirdPersonController.instance.UpdatehealthBar();
                PlayerAudioManager.instance.UseItem.Play();

            }
        }

    }

    void OnCollisionExit(PhysicsComponent rb)
    {
        //detect the player
        if (GetTag(rb) == "Player")
        {
            Console.WriteLine("AwayFromCheckpoint");
            ThirdPersonController.instance.isAtCheckpoint = false;
        }
    }
}
