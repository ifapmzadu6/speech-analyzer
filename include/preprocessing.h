#pragma once

#include <iostream>

class Preproccesing {
    // 正規化
    // 全てのサンプルの平均が０になるように調整
    // その後,標準偏差で割り算をし、成分ごとの分散を１にする
    void standardization();

    // ガウス分布に従うランダムノイズを加える
    // 幾何学的変形
    // 鏡面反射
    void augmentation();

    // Ada Grid
    // Momentum

    // バイアスの初期値は0が普通
};
