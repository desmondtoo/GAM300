﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using BeanFactory;

public class DialogueManagerBoss : Script 
{
    public static DialogueManagerBoss Instance;

    public GameObject dialogueText;
    TextRenderer text;

    //public AudioSource seer1;
    //public AudioSource seer2;
    //public AudioSource seer3;
    //public AudioSource seer4;

    public AudioSource apex1;
    public AudioSource apex2;
    public AudioSource apex3;
    public AudioSource apex4;
    public AudioSource apex5;
    public AudioSource apex6;
    public AudioSource apex7;
    public AudioSource apex8;
    public AudioSource apex9;
    public AudioSource apex10;
    public AudioSource apex11;
    public AudioSource apex12;
    public AudioSource apex13;

    bool startTimer;
    float Timer;
    //int curr_state = 0;

    void Awake()
    {
        Instance = this;
        startTimer = false;
        text = dialogueText.GetComponent<TextRenderer>();
    }

    void Update()
    {
        if (startTimer)
        {
            if (Timer > 0)
            {
                Timer -= Time.deltaTime;
            }
            else
            {
                startTimer = false;
                dialogueText.SetActive(false);
            }
        }
    }

    void setTimer (float duration)
    {
        startTimer = true;
        Timer = duration;
    }

    public void SetState(int i)
    {
        dialogueText.SetActive (true);
        switch (i)
        {
            case 1:
                text.text = "APEX: So, you're finally here? Allow me to demonstrate the gulf in quality that separates us.";
                apex1.Play();
                setTimer(8f);              
                break;
            case 2:
                text.text = "APEX: Remarkable...Unfortunately, I've gathered enough data now and it's time to end this!";
                apex2.Play();
                setTimer(8f);
                break;
            case 3:
                text.text = "APEX: No! Impossible... The Mission... It can't end like this...";
                apex3.Play();
                setTimer(7f);
                break;
            case 4:
                text.text = "APEX: Let me show you what fast looks like!";
                apex4.Play();
                setTimer(4f);
                break;
            case 5:
                text.text = "Apex: Don't think you can run!";
                apex5.Play();
                setTimer(5f);
                break;
            case 6:
                text.text = "Apex: Is that the best you can do???";
                apex6.Play();
                setTimer(4f);
                break;
            case 7:
                text.text = "Apex: You're just a cheap knockoff!";
                apex7.Play();
                setTimer(3f);
                break;
            case 8: 
                text.text = "Apex: Dodge this!";
                apex8.Play();
                setTimer(1.5f);
                break;
            case 9: 
                text.text = "Apex: There's no escape!";
                apex9.Play();
                setTimer(1.4f);
                break;
            case 10: 
                text.text = "Apex: Catch me if you can.";
                apex10.Play();
                setTimer(1.5f);
                break;
            case 11:
                text.text = "Apex: HAHAHAHA!!!";
                apex11.Play();
                setTimer(2.2f);
                break;
            case 12:
                text.text = "Apex: What a joke!";
                apex12.Play();
                setTimer(2.2f);
                break;
            case 13:
                text.text = "Apex: Did I overestimate your abilities?";
                apex13.Play();
                setTimer(2.4f);
                break;
        }
    }
}
