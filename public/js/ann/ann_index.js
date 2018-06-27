"use strict";

const fs = require("fs");
const { multiply, sigma, err, isArray, Random } = require("./utils.js");
let rnd = Random(1);

module.exports = NeuralNetwork;

/**
 * Constructor for the Neural Network
 * @namespace NeuralNetwork
 * @param {integer} numInputs - number of input neurons
 * @param {integer} numHidden - number of neurons in hidden layer
 * @param {integer} numOutputs - number of output neurons
 */
function NeuralNetwork(numInputs, numHidden, numOutputs, learningRate = 0.5, bias = 1) {
  let weightsIH = getStartingWeights(numInputs + 1, numHidden);
  let weightsHO = getStartingWeights(numHidden + 1, numOutputs);


  /**
   * Saves the current weights of the neural network in a json file
   * @memberof NeuralNetwork
   * @param {string} file - the target file
   * @param {function} cb - callback
   */
  function save(file, cb) {
    let data = {
      weightsIH,
      weightsHO
    };
    fs.writeFile(file, JSON.stringify(data), cb);
  }


  /**
   * Saves the current weights of the neural network synchronously in a json file
   * @memberof NeuralNetwork
   * @param {string} file - the target file
   * @param {function} cb - callback
   */
  function saveSync(file) {
    let data = {
      weightsIH,
      weightsHO
    };
    fs.writeFileSync(file, JSON.stringify(data));
  }

  /**
   * Loads the weights from a json file
   * @memberof NeuralNetwork
   * @param {string} file - the target file
   */
  function load(file, cb) {
    fs.readFile(file, (err, data) => {
      if (err) {
        return cb(err);
      }
      data = JSON.parse(data);
      ({ weightsIH, weightsHO } = data.weights);

      cb();
    });
  }

  /**
   * Loads the weights from a json file synchronously
   * @memberof NeuralNetwork
   * @param {string} file - the target file
   */
  function loadSync(file) {
    ({ weightsIH, weightsHO } = JSON.parse(fs.readFileSync(file)));
  }


  function feedForward(input) {
    let netH = multiply([input.concat([bias])], weightsIH)[0];
    let outH = netH.map(c => sigma(c));
    let netO = multiply([outH.concat([bias])], weightsHO)[0];
    let outO = netO.map(c => sigma(c));

    return {
      outH,
      outO
    };
  }

  /**
   * Feeds the network with the input and backpropagates the error
   * @memberof NeuralNetwork
   * @param {Object} data - the target file
   * @param {Array} data.input - the input vector
   * @param {Array} data.expected - the expected result
   */
  function train({ input, expected }) {
    input = isArray(input) ? input : [input];
    expected = isArray(expected) ? expected : [expected];

    const {
      outH,
      outO
    } = feedForward(input);

    const errors = err(outO, expected);
    const totalError = errors.reduce((p, c) => {
      return p + c;
    }, 0);

    console.log("total error", totalError);

    const dErrordOutH = [];
    for (let i = 0; i < numHidden; i++) {
      let sum = 0;
      for (let j = 0; j < numOutputs; j++) {
        sum += (outO[j] - expected[j]) * outO[j] * (1 - outO[j]) * weightsHO[i][j];
      }
      dErrordOutH.push(sum);
    }

    for (let i = 0; i < numHidden + 1; i++) {
      for (let j = 0; j < numOutputs; j++) {
        let deltaW = (outO[j] - expected[j]) * outO[j] * (1 - outO[j]);
        if (i !== numHidden) {
          deltaW *= outH[i];
        } else {
          deltaW *= bias;
        }
        weightsHO[i][j] -= deltaW * learningRate;
      }
    }

    for (let i = 0; i < numInputs + 1; i++) {
      for (let j = 0; j < numHidden; j++) {
        let deltaW = dErrordOutH[j] * outH[j] * (1 - outH[j]);
        if (i !== numInputs) {
          deltaW *= input[i];
        } else {
          deltaW *= bias;
        }
        weightsIH[i][j] -= deltaW * learningRate;
      }
    }
  }

  /**
   * Returns result of a test input
   * @memberof NeuralNetwork
   * @param {Array} input - the test input vector
   */
  function test(input) {
    const result = feedForward(isArray(input) ? input : [input]).outO;

    return result.length === 1 ? result[0] : result;
  }

  return {
    train,
    test,
    save,
    saveSync,
    load,
    loadSync
  };
}

/**
 * Returns an m x n matrix of random starting weights
 * @private
 * @param {integer} m - the number of rows
 * @param {integer} n -  the number of columns
 */
function getStartingWeights(m, n) {
  let matrix = [];

  for (let i = 0; i < m; i++) {
    let row = [];
    for (let j = 0; j < n; j++) {
      row.push(rnd.next(-1, 1));
    }
    matrix.push(row);
  }

  return matrix;
}
