﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BeanFactory;
using GlmSharp;

public class AudioManager : Script
{
    public static AudioManager instance;

    //ADD A GAMEOBJECT FOR EACH SOUND
    public AudioSource playerSlashAttack;
    public AudioSource playerInjured;
    public AudioSource jumpVoice;
    public AudioSource spark;
    public AudioSource itemCollected;
    public AudioSource rangeEnemyFiring;
    public AudioSource rangeEnemyDead;
    public AudioSource doorOpen;
    public AudioSource elevator;
    public AudioSource playerFootstep;
    public AudioSource playerAttack;

    void Awake()
    {
        if (instance != null)
        {
            //Debug.LogError("More than one AudioManager in the scene.");
        }
        else
        {
            instance = this;
        }
    }

    void Start()
    {

    }
}