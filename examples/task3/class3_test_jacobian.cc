//// Created by caoqi on 2018/8/31.////3D:  1.36939, -1.17123, 7.04869//obs: 0.180123 -0.156584#include "sfm/bundle_adjustment.h"/* * This function computes the Jacobian entries for the given camera and * 3D point pair that leads to one observation. * * The camera block 'cam_x_ptr' and 'cam_y_ptr' is: * - ID 0: Derivative of focal length f * - ID 1-2: Derivative of distortion parameters k0, k1 * - ID 3-5: Derivative of translation t0, t1, t2 * - ID 6-8: Derivative of rotation w0, w1, w2 * * The 3D point block 'point_x_ptr' and 'point_y_ptr' is: * - ID 0-2: Derivative in x, y, and z direction. * * The function that leads to the observation is given as follows: * *   u = f * D(x,y) * x  (image observation x coordinate) *   v = f * D(x,y) * y  (image observation y coordinate) * * with the following definitions: * *   xc = R0 * X + t0  (homogeneous projection) *   yc = R1 * X + t1  (homogeneous projection) *   zc = R2 * X + t2  (homogeneous projection) *   x = xc / zc  (central projection) *   y = yc / zc  (central projection) *   D(x, y) = 1 + k0 (x^2 + y^2) + k1 (x^2 + y^2)^2  (distortion) */ /**  * /description 给定一个相机参数和一个三维点坐标，求解雅各比矩阵，即公式中的df(theta)/dtheta  * @param cam       相机参数  * @param point     三维点坐标  * @param cam_x_ptr 重投影坐标x 相对于相机参数的偏导数，相机有9个参数： [0] 焦距f; [1-2] 径向畸变系数k1, k2; [3-5] 平移向量 t1, t2, t3  *                                                               [6-8] 旋转矩阵（角轴向量）  * @param cam_y_ptr    重投影坐标y 相对于相机参数的偏导数，相机有9个参数  * @param point_x_ptr  重投影坐标x 相对于三维点坐标的偏导数  * @param point_y_ptr  重投影坐标y 相对于三维点坐标的偏导数  */void jacobian(sfm::ba::Camera const& cam,              sfm::ba::Point3D const& point,              double* cam_x_ptr, double* cam_y_ptr,              double* point_x_ptr, double* point_y_ptr){    const double f = cam.focal_length;    const double *R = cam.rotation;    const double *t = cam.translation;    const double *X = point.pos;    const double k0 = cam.distortion[0];    const double k1 = cam.distortion[1];    //inline d_(r2) (1+(k0+k1*r2)*r2);    double xc[3];    xc[0] = R[0]*X[0] + R[1]*X[1] + R[2]*X[2] + t[0];    xc[1] = R[3]*X[0] + R[4]*X[1] + R[5]*X[2] + t[1];    xc[2] = R[6]*X[0] + R[7]*X[1] + R[8]*X[2] + t[2];    //归一化平面    double x[2];    x[0] = xc[0]/xc[2];    x[1] = xc[1]/xc[2];    //像平面    double r2 = x[0]*x[0] + x[1]*x[1];    double d_ = 1+(k0+k1*r2)*r2;    double u = f*d_*x[0];    double v = f*d_*x[1];    // 相机焦距的偏导数    cam_x_ptr[0] = d_*x[0];    cam_y_ptr[0] = d_*x[1];    // 相机径向畸变的偏导数    cam_x_ptr[1] = f*x[0]*r2;    cam_x_ptr[2] = f*x[0]*r2*r2;    cam_y_ptr[1] = f*x[1]*r2;    cam_y_ptr[2] = f*x[1]*r2*r2;    // 相机将向畸变系数的偏导数//    cam_x_ptr[1] = 0.0;//    cam_x_ptr[2] = 0.0;//    cam_y_ptr[1] = 0.0;//    cam_y_ptr[2] = 0.0;    // 相机平移向量的偏导数    double d1 = k0+2*k1*r2;    cam_x_ptr[3] = f*x[0]*d1*2*x[0]/xc[2]+f*d_/xc[2];    cam_x_ptr[4] = f*x[0]*d1*2*x[1]/xc[2];    cam_x_ptr[5] = f*x[0]*(-d1)*2*r2/xc[2]+f*d_*(-x[0])/xc[2];    cam_y_ptr[3] = f*x[1]*d1*2*x[0]/xc[2];    cam_y_ptr[4] = f*x[1]*d1*2*x[1]/xc[2]+f*d_/xc[2];    cam_y_ptr[5] = f*x[1]*(-d1)*2*r2/xc[2]+f*d_*(-x[1])/xc[2];    // 相机旋转矩阵的偏导数    double xc_w0 = 0;    double xc_w1 = R[6]*X[0] + R[7]*X[1] + R[8]*X[2];    double xc_w2 = -(R[3]*X[0] + R[4]*X[1] + R[5]*X[2]);    double yc_w0 = -xc_w1;    double yc_w1 = 0;    double yc_w2 = R[0]*X[0] + R[1]*X[1] + R[2]*X[2];    double zc_w0 = -xc_w2;    double zc_w1 = -yc_w2;    double zc_w2 = 0;    double u_xc = cam_x_ptr[3];    double u_yc = cam_x_ptr[4];    double u_zc = cam_x_ptr[5];    double v_xc = cam_y_ptr[3];    double v_yc = cam_y_ptr[4];    double v_zc = cam_y_ptr[5];    cam_x_ptr[6] = cam_x_ptr[4]*yc_w0+cam_x_ptr[5]*zc_w0;    cam_x_ptr[7] = cam_x_ptr[3]*xc_w1+cam_x_ptr[5]*zc_w1;    cam_x_ptr[8] = cam_x_ptr[3]*xc_w2+cam_x_ptr[4]*yc_w2;    cam_y_ptr[6] = cam_y_ptr[4]*yc_w0+cam_y_ptr[5]*zc_w0;    cam_y_ptr[7] = cam_y_ptr[3]*xc_w1+cam_y_ptr[5]*zc_w1;    cam_y_ptr[8] = cam_y_ptr[3]*xc_w2+cam_y_ptr[4]*yc_w2;    // 三维点的偏导数    double xc_X = R[0];    double xc_Y = R[1];    double xc_Z = R[2];    double yc_X = R[3];    double yc_Y = R[4];    double yc_Z = R[5];    double zc_X = R[6];    double zc_Y = R[7];    double zc_Z = R[8];    point_x_ptr[0] = u_xc*xc_X+u_yc*yc_X+u_zc*zc_X;    point_x_ptr[1] = u_xc*xc_Y+u_yc*yc_Y+u_zc*zc_Y;    point_x_ptr[2] = u_xc*xc_Z+u_yc*yc_Z+u_zc*zc_Z;    point_y_ptr[0] = v_xc*xc_X+v_yc*yc_X+v_zc*zc_X;    point_y_ptr[1] = v_xc*xc_Y+v_yc*yc_Y+v_zc*zc_Y;    point_y_ptr[2] = v_xc*xc_Z+v_yc*yc_Z+v_zc*zc_Z;}int main(int argc, char*argv[]){    sfm::ba::Camera cam;    cam.focal_length  =  0.919654;    cam.distortion[0] = -0.108298;    cam.distortion[1] =  0.103775;    cam.rotation[0] = 0.999999;    cam.rotation[1] = -0.000676196;    cam.rotation[2] = -0.0013484;    cam.rotation[3] = 0.000663243;    cam.rotation[4] = 0.999949;    cam.rotation[5] = -0.0104095;    cam.rotation[6] = 0.00135482;    cam.rotation[7] = 0.0104087;    cam.rotation[8] = 0.999949;    cam.translation[0]=0.00278292;    cam.translation[1]=0.0587996;    cam.translation[2]=-0.127624;    sfm::ba::Point3D pt3D;    pt3D.pos[0]= 1.36939;    pt3D.pos[1]= -1.17123;    pt3D.pos[2]= 7.04869;    double cam_x_ptr[9]={0};    double cam_y_ptr[9]={0};    double point_x_ptr[3]={0};    double point_y_ptr[3]={0};    jacobian(cam, pt3D, cam_x_ptr, cam_y_ptr, point_x_ptr, point_y_ptr);   std::cout<<"Result is :"<<std::endl;    std::cout<<"cam_x_ptr: ";    for(int i=0; i<9; i++){        std::cout<<cam_x_ptr[i]<<" ";    }    std::cout<<std::endl;    std::cout<<"cam_y_ptr: ";    for(int i=0; i<9; i++){        std::cout<<cam_y_ptr[i]<<" ";    }    std::cout<<std::endl;    std::cout<<"point_x_ptr: ";    std::cout<<point_x_ptr[0]<<" "<<point_x_ptr[1]<<" "<<point_x_ptr[2]<<std::endl;    std::cout<<"point_y_ptr: ";    std::cout<<point_y_ptr[0]<<" "<<point_y_ptr[1]<<" "<<point_y_ptr[2]<<std::endl;    std::cout<<"\nResult should be :\n"       <<"cam_x_ptr: 0.195942 0.0123983 0.000847141 0.131188 0.000847456 -0.0257388 0.0260453 0.95832 0.164303\n"       <<"cam_y_ptr: -0.170272 -0.010774 -0.000736159 0.000847456 0.131426 0.0223669 -0.952795 -0.0244697 0.179883\n"       <<"point_x_ptr: 0.131153 0.000490796 -0.0259232\n"       <<"point_y_ptr: 0.000964926 0.131652 0.0209965\n";    return 0;}