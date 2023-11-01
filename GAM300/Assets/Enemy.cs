﻿using BeanFactory;
using System;
using System.Collections.Generic;
using System.Collections;
using GlmSharp;

public class Enemy : Script
{
    public float moveSpeed = 2f;
    public float chaseDistance = 5f;
    public Transform player;
    public int count;

    //public float ching;


    void Start()
    {
        Console.WriteLine("START");
        moveSpeed = 2f;
        //AddComponent<Rigidbody>();
        //rb = GetComponent<Rigidbody>();
        
    }

    void Update()
    {
        //follow target
        vec3 direction = player.localPosition - transform.localPosition;
        //double angle = Math.Atan2(direction.x, direction.z);
        //vec3.Distance(player.localPosition, this.transform.localPosition);

        if(vec3.Distance(player.localPosition, this.transform.localPosition) <= chaseDistance)
        {
            GetComponent<Rigidbody>().linearVelocity = new vec3(direction.Normalized) * moveSpeed;
        }
    }

    void Exit()
    {

    }
}

