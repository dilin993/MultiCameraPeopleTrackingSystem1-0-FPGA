//
// Created by dilin on 11/3/17.
//

#include "ParticleFilterTracker.h"

State::State(double x,double y,double vx,double vy):
x(x),
y(y),
vx(vx),
vy(vy)
{
}

State::State() :
State(0,0,0,0)
{
}

void State::set(double x, double y, double vx, double vy)
{
    this->x = x;
    this->y = y;
    this->vx = vx;
    this->vy = vy;
}

ParticleFilterTracker::ParticleFilterTracker(Point2f detection,
                                             MatND histogram,
                                             double sigma_propagate[],
                                             unsigned int noParticles,
                                             double alpha,
                                             double var_m,
                                             double hc) :
noParticles(noParticles),
particles(noParticles),
particles_new(noParticles),
weights(noParticles),
alpha(alpha),
var_m(var_m),
hc(hc)
{
    double init = 1.0/noParticles;
    for(int i=0;i<noParticles;i++)
    {
        particles[i] = State(detection.x,detection.y,0,0);
        particles_new[i] = State(detection.x,detection.y,0,0);
        weights[i] = init;
    }

    for(int i=0;i<NUM_STATES;i++)
    {
        this->sigma_propagate[i] = sigma_propagate[i];
    }

    this->histogram = histogram;

    pastStates.push(State(detection.x,detection.y,0,0));

    color = cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
}

void ParticleFilterTracker::predict()
{
    vector<double> random(NUM_STATES);

    random_device rd;
    mt19937 gen(rd());
    for (unsigned int index = 0; index < noParticles; index++)
    {
        for (int j = 0; j < NUM_STATES; j++)
        {
            normal_distribution<> d(0, sigma_propagate[j]);
            random[j] = d(gen);
        }

        particles_new[index].x = particles[index].x + random[0];
        particles_new[index].y = particles[index].y + random[1];
        particles_new[index].vx = particles[index].vx + random[2];
        particles_new[index].vy = particles[index].vy + random[3];
    }
}

void ParticleFilterTracker::measurement(Point2f detection, MatND histogram)
{
    double constGaus = 1.0/sqrt(2*M_PI*var_m);
    for(int i=0;i<noParticles;i++)
    {
        double dist = pow(particles[i].x-detection.x,2.0);
        dist += pow(particles[i].y-detection.y,2.0);
        dist /= (2.0*var_m);
        weights[i] = constGaus * exp(-dist);
    }
}


void ParticleFilterTracker::systematicResampling(vector<double>& w, vector<int>& indx)
{
    // random from std
    random_device rd;
    mt19937 gen(rd());
    //initialize
    vector<double> Q(noParticles,0.0);
    vector<double> Wnorm(noParticles,0.0);
    // normalize w
    double sum_w = 0.0;
    for (unsigned int i =0 ;  i  < noParticles; i++)
    {
        sum_w += w.at(i);
    }
    for (unsigned int i =0 ;  i  < noParticles; i++)
    {
        Wnorm.at(i) = w.at(i) / sum_w;
    }
    // cum sum
    sum_w = 0.0;
    for (unsigned int i =0 ;  i  < noParticles; i++) {
        sum_w += Wnorm.at(i);
        Q.at(i) = sum_w;
    }
    Q.at(noParticles-1)=1.;
    // Uniform random vector
    vector<double> T(noParticles+1);
    uniform_real_distribution<> dis(0, 1);
    for (unsigned int i = 0; i < noParticles; i++) {
        T.at(i) = (double(i) + dis(gen))/noParticles;

    }
    T.at(noParticles) = 1;
    // resampling
    unsigned int i = 0, j = 0;
    while (i < noParticles) {
        if (T.at(i) < Q.at(j) && Q.at(j) > 0.4)
        {
            indx.at(i) = j;
            i++;
        }
        else
        {
            j++;
            if (j >= noParticles) {
                j = noParticles - 1;
            }
        }
    }
}

void ParticleFilterTracker::updateState( vector<double> normalized_weights)
{
    //weighted mean pos
    double xpos = 0.0, ypos = 0.0, xvel = 0.0, yvel = 0.0;
    for (unsigned int i = 0; i < noParticles; i++) {
        double tmp_x = particles_new[i].x * normalized_weights.at(i);
        double tmp_y = particles_new[i].y * normalized_weights.at(i);
        double tmp_vx = particles_new[i].vx * normalized_weights.at(i);
        double tmp_vy = particles_new[i].vy * normalized_weights.at(i);

        xpos += tmp_x;
        ypos += tmp_y;
        xvel += tmp_vx;
        yvel += tmp_vy;
    }

    if(pastStates.size() >= MAX_PAST)
        pastStates.pop();
    pastStates.push(State(xpos,ypos,xvel,yvel));
}

void ParticleFilterTracker::assignDetection(Point2f detection, MatND histogram)
{
    measurement(detection,histogram);
    vector<int> indx(noParticles,0);
    for (unsigned int i = 0; i < noParticles; i++)
    {
        indx.at(i) = i;
    }
    systematicResampling(weights, indx);
    for (unsigned int i =0; i < noParticles; i++)
    {
        int id = indx[i];
        particles[i].x = particles_new[id].x;
        particles[i].y = particles_new[id].y;
        particles[i].vx = particles_new[id].vx;
        particles[i].vy = particles_new[id].vy;
    }

    vector<double> normalized_weights(noParticles,0.0);
    double w_sum = 0.0;
    for (unsigned int i = 0; i < noParticles; i++) {
        w_sum += weights.at(i);
    }
    if(w_sum==0.0)
    {
        initialize(detection);
        return;
    }
    for (unsigned int i = 0; i < noParticles; i++)
    {
        normalized_weights[i] = weights.at(i)/w_sum;
    }

    updateState(normalized_weights);
    this->histogram = (1.0-hc)*this->histogram + hc*histogram;
}

void ParticleFilterTracker::update()
{
    predict();
    //updateState();
}

cv::RNG ParticleFilterTracker::rng = cv::RNG(12345);

Point2f ParticleFilterTracker::getPos()
{
    if(pastStates.empty())
        return Point2f(0,0);
    State cur =  pastStates.back();
    return Point2f((int)cur.x,(int)cur.y);
}

State ParticleFilterTracker::getState() {

    if(pastStates.empty())
        return State(0,0,0,0);
    return pastStates.back();
}

void ParticleFilterTracker::updateAssociation(bool detectionAssociated)
{
    age++;
    if(detectionAssociated)
    {
        totalVisibleCount += 1;
    }
    else if(!prevDetectionAssociated)
    {
        consectiveInvisibleCount++;
    }

    prevDetectionAssociated = detectionAssociated;
}

void ParticleFilterTracker::initialize(Point2f detection)
{
    double init = 1.0/noParticles;
    for(int i=0;i<noParticles;i++)
    {
        particles[i] = State(detection.x,detection.y,0,0);
        particles_new[i] = State(detection.x,detection.y,0,0);
        weights[i] = init;
    }
    if(pastStates.size() >= MAX_PAST)
        pastStates.pop();
    pastStates.push(State(detection.x,detection.y,0,0));
}
