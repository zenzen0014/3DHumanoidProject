"use strict";

module.exports = {
  multiply,
  sigma,
  err,
  isArray,
  Random
};


/**
 * square error measure
 * @param {Array} actual - actual output vector
 * @param {Array} expected - expected output vector
 * @returns {Array} an array of squared error for each vector element
 */
function err(actual, expected) {
  return actual.map((c, i) => {
    return 0.5 * Math.pow((expected[i] - actual[i]), 2);
  });
}

/**
 * activation function
 * @param {number} x - the x value
 */
function sigma(x) {
  return 1 / (1 + Math.exp(-x));
}

/**
 * multiply two matrices
 * @param {Array} leftMatrix - the left matrix
 * @param {Array} rightMatrix - the right matrix
 * @returns {Array} a Matrix that is a result of the multiplication
 */
function multiply(leftMatrix, rightMatrix) {
  const rows = leftMatrix.length;
  const cols = rightMatrix[0].length;
  const dotLength = rightMatrix.length;
  let result = [];

  for (let i = 0; i < rows; i++) {
    let row = [];
    for (let j = 0; j < cols; j++) {
      let dot = 0;
      for (let k = 0; k < dotLength; k++) {
        dot += leftMatrix[i][k] * rightMatrix[k][j];
      }
      row.push(dot);
    }
    result.push(row);
  }
  return result;
}

/**
 * Custom random number generator,
 * that outputs the same sequence of numbers if the seed does not change
 * @param {number} seed - the seed for the generator
 */
function Random(seed) {
  function next(min = 0, max = 1) {
    const x = Math.sin(seed++) * 10000;
    return (x - Math.floor(x)) * (max - min) + min;
  }

  function nextInt(min = 0, max = 1) {
    return Math.floor(next() * (max - min + 1) + min);
  }

  return {
    next,
    nextInt
  };
}


/**
 * Checks whether the argument is an array
 * @param {Array|number} the tested subject
 *
 */
function isArray(subject) {
  return Array.isArray(subject);
}
