﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BeanFactory;
using GlmSharp;

public class PlatformScript : Script
{
    public Transform startPoint;
    public Transform endPoint;
    public float duration = 2f;
    public float timer = 0f;
    public bool back = false;

    public bool rest = false;
    public float restTimer = 2f;
    public float currentRestTimer;

    CharacterController player;
    public AudioSource soundEffect;

    public ThirdPersonController thirdPersonController;

    void Start()
    {
        currentRestTimer = restTimer;
        //soundEffect.Play();
    }

    void Update()
    {
        if (Input.GetKeyUp(KeyCode.G))
        {
            bool val = !IsEnabled(soundEffect);
            SetEnabled(soundEffect, val);
            Console.WriteLine("Sound effect " + val);
        }
        vec3 target = transform.localPosition;
        vec3 diff = vec3.Zero; 
        if (!back && !rest)
        {
            target = vec3.Lerp(startPoint.localPosition, endPoint.localPosition, timer / duration);
            diff = target - transform.localPosition;
        }
        else if(back && !rest)
        {
            target = vec3.Lerp(endPoint.localPosition, startPoint.localPosition, timer / duration);
            diff = target - transform.localPosition;
        }
        transform.localPosition = target;
/*        if (player != null)
        {
            player.Move(diff * 60f);
        }*/

        timer += Time.deltaTime;
        if (timer >= duration)
        {
            rest = true;//stops platform from moving
            //Console.WriteLine("PlatformResting");
        }
        if(rest)
        {
            currentRestTimer -= Time.deltaTime;
            if(currentRestTimer <= 0)
            {
                //Console.WriteLine("PlatformStartMoving");
                rest = false;
                currentRestTimer = restTimer;//rest rest timer
                back = !back;//change to the other movement direction
                timer = 0f;//rest movement timer
            }
        }
    }
    void OnCollisionEnter(PhysicsComponent rb)
    {
        //detect the player
        if (GetTag(rb) == "Player")
        {
            player = rb.gameObject.GetComponent<CharacterController>();
            player.gameObject.transform.SetParent(transform);
            Console.WriteLine("PlayerOnPlatform");
        }
    }

    void OnCollisionExit(PhysicsComponent rb)
    {
        //detect the player
        if (GetTag(rb) == "Player")
        {
            player.gameObject.transform.SetParent(null);
            player = null;
            Console.WriteLine("Player Exit Platform");
        }
    }
}
